#include "VendingApplicationController.h"

#include <ICardReader.h>
#include <Transaction.h>
#include <TransactionRepository.h>
#include <VendingMachine.h>
#include <ITransactionTransport.h>
#include <SyncWorker.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>
#include <QTimer>
#include <QUrl>

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <utility>

class VendingApplicationController::Dispenser final : public IDispenser
{
public:
    void dispense(const ProductId&) override {}
};

class HttpTransactionTransport final : public ITransactionTransport
{
public:
    TransportResult postTransaction(const PostTransactionRequest& request) override
    {
        QNetworkAccessManager manager;
        const auto baseUrl = qEnvironmentVariable("VENDING_BACKEND_URL", "http://127.0.0.1:8080");
        QNetworkRequest networkRequest{QUrl{baseUrl + QString::fromStdString(request.endpoint)}};
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        networkRequest.setRawHeader("Idempotency-Key", QByteArray::fromStdString(request.idempotencyKey));

        QNetworkReply* reply = manager.post(networkRequest, QByteArray::fromStdString(request.jsonBody));
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        timeout.start(2'000);
        loop.exec();

        if (!reply->isFinished()) {
            reply->abort();
            reply->deleteLater();
            return TransportResult::RetryableFailure;
        }

        const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        reply->deleteLater();
        return statusCode >= 200 && statusCode < 300 ? TransportResult::Success
                                                      : TransportResult::RetryableFailure;
    }
};

class VendingApplicationController::SyncRunner final : public QObject
{
public:
    explicit SyncRunner(QString databasePath)
        : databasePath(std::move(databasePath))
    {}

    void start()
    {
        try {
            repository = std::make_unique<TransactionRepository>(databasePath.toStdString());
            repository->initialize();
            transport = std::make_unique<HttpTransactionTransport>();
            worker = std::make_unique<SyncWorker>(*repository, *transport);
        }
        catch (...) {
            isOnline.store(false);
            return;
        }

        timer = new QTimer(this);
        timer->setInterval(1'000);
        QObject::connect(timer, &QTimer::timeout, this, [this] { runOnce(); });
        timer->start();
        runOnce();
    }

    [[nodiscard]] bool online() const
    {
        return isOnline.load();
    }

private:
    void runOnce()
    {
        if (!worker) {
            return;
        }

        const auto result = worker->runOnce(SyncWorker::Clock::now());
        if (result.attempted > 0) {
            isOnline.store(result.synchronized == result.attempted);
        }
    }

    QString databasePath;
    QTimer* timer{nullptr};
    std::unique_ptr<TransactionRepository> repository;
    std::unique_ptr<HttpTransactionTransport> transport;
    std::unique_ptr<SyncWorker> worker;
    std::atomic_bool isOnline{false};
};

namespace
{
QString stateToString(const VendingState state)
{
    switch (state) {
        case VendingState::Idle:
            return "Idle";
        case VendingState::CardRead:
            return "CardRead";
        case VendingState::ProductSelected:
            return "ProductSelected";
        case VendingState::Dispensing:
            return "Dispensing";
        case VendingState::Completed:
            return "Completed";
        case VendingState::Failed:
            return "Failed";
    }
    return "Unknown";
}
} // namespace

VendingApplicationController::VendingApplicationController(QObject* parent)
    : QObject(parent)
    , dispenser(std::make_unique<Dispenser>())
    , repository(std::make_unique<TransactionRepository>(
          QDir(QCoreApplication::applicationDirPath()).filePath("transactions.sqlite").toStdString()))
    , ownedMachine(std::make_unique<VendingMachine>(*dispenser))
    , machine(ownedMachine.get())
{
    setupSelectionTimeout();
    repository->initialize();
    refreshState();
    refreshPendingTransactions();
    startSynchronization();
}

VendingApplicationController::VendingApplicationController(ICardReader& cardReader,
                                                           VendingMachine& machine,
                                                           QObject* parent)
    : QObject(parent)
    , dispenser(nullptr)
    , repository(std::make_unique<TransactionRepository>())
    , machine(&machine)
{
    setupSelectionTimeout();
    repository->initialize();
    connectCardReader(cardReader);
    refreshState();
    refreshPendingTransactions();
}

VendingApplicationController::~VendingApplicationController()
{
    stopSynchronization();
    if (cardReader) {
        cardReader->setCardTappedHandler({});
    }
}

QString VendingApplicationController::state() const
{
    return lastState;
}

int VendingApplicationController::pendingTransactions() const
{
    return pendingCount;
}

bool VendingApplicationController::online() const
{
    return isOnline;
}

void VendingApplicationController::simulateCardTap()
{
    if (machine->onCardTapped(CardId{"demo-card"}) == EventResult::Accepted) {
        startSelectionTimeout();
    }
    refreshState();
}

void VendingApplicationController::selectProduct(const QString& productId)
{
    if (!machine->getCardId().has_value()) {
        return;
    }

    const auto product = productId.toStdString();
    const auto transaction = Transaction::create(QDateTime::currentMSecsSinceEpoch(),
                                                 product,
                                                 machine->getCardId()->value,
                                                 Status::Dispensing);

    try {
        // The journal entry is durable before the simulated dispenser starts.
        repository->insert(transaction);
        activeTransactionId = QString::fromStdString(transaction.id);
        refreshPendingTransactions();

        if (machine->onProductSelected(ProductId{product}) == EventResult::Accepted) {
            stopSelectionTimeout();
            refreshState();
            QTimer::singleShot(1500, this, [this] {
                finishDispensing(true);
            });
        }
    }
    catch (const std::exception& error) {
        emit errorOccurred(QString::fromUtf8(error.what()));
    }
}

void VendingApplicationController::reset()
{
    stopSelectionTimeout();
    machine->reset();
    refreshState();
}

void VendingApplicationController::simulateDispenseFailure()
{
    finishDispensing(false);
}

void VendingApplicationController::finishDispensing(const bool success)
{
    if (!machine->getProductId().has_value()) {
        return;
    }

    const auto result = machine->onDispenseResult(success ? DispenseResult::Success : DispenseResult::Failure);
    if (result != EventResult::Accepted) {
        return;
    }

    if (!activeTransactionId.isEmpty()) {
        repository->updateStatus(activeTransactionId.toStdString(), success ? Status::Completed : Status::Failed);
        activeTransactionId.clear();
    }
    refreshState();
    refreshPendingTransactions();
}

void VendingApplicationController::refreshState()
{
    const auto newState = stateToString(machine->getState());
    if (lastState == newState) {
        return;
    }
    lastState = newState;
    emit stateChanged();
}

void VendingApplicationController::refreshPendingTransactions()
{
    const auto newCount = static_cast<int>(repository->findUnsynchronized().size());
    if (pendingCount == newCount) {
        return;
    }
    pendingCount = newCount;
    emit pendingTransactionsChanged();
}

void VendingApplicationController::connectCardReader(ICardReader& reader)
{
    cardReader = &reader;
    cardReader->setCardTappedHandler([this](CardId cardId) {
        if (machine->onCardTapped(std::move(cardId)) == EventResult::Accepted) {
            startSelectionTimeout();
        }
        refreshState();
    });
}

void VendingApplicationController::startSelectionTimeout()
{
    selectionTimeoutTimer.start(15'000);
}

void VendingApplicationController::stopSelectionTimeout()
{
    selectionTimeoutTimer.stop();
}

void VendingApplicationController::startSynchronization()
{
    const auto databasePath = QDir(QCoreApplication::applicationDirPath())
                                  .filePath("transactions.sqlite");
    syncThread = std::make_unique<QThread>();
    syncRunner = new SyncRunner(databasePath);
    syncRunner->moveToThread(syncThread.get());

    connect(syncThread.get(), &QThread::started, syncRunner, [runner = syncRunner] {
        runner->start();
    });
    connect(syncThread.get(), &QThread::finished, syncRunner, &QObject::deleteLater);
    syncThread->start();

    synchronizationStatusTimer.setInterval(500);
    connect(&synchronizationStatusTimer, &QTimer::timeout, this, [this] {
        refreshSynchronizationStatus();
        refreshPendingTransactions();
    });
    synchronizationStatusTimer.start();
}

void VendingApplicationController::stopSynchronization()
{
    synchronizationStatusTimer.stop();
    if (syncThread) {
        syncThread->quit();
        syncThread->wait();
        syncThread.reset();
        syncRunner = nullptr;
    }
}

void VendingApplicationController::refreshSynchronizationStatus()
{
    const auto newOnline = syncRunner && syncRunner->online();
    if (isOnline == newOnline) {
        return;
    }
    isOnline = newOnline;
    emit onlineChanged();
}

void VendingApplicationController::setupSelectionTimeout()
{
    selectionTimeoutTimer.setSingleShot(true);
    selectionTimeoutTimer.setInterval(15'000);
    connect(&selectionTimeoutTimer, &QTimer::timeout, this, [this] {
        if (machine->onSelectionTimeout() == EventResult::Accepted) {
            refreshState();
        }
    });
}
