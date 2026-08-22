#include "VendingApplicationController.h"

#include <ICardReader.h>
#include <Transaction.h>
#include <TransactionRepository.h>
#include <VendingMachine.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QTimer>

#include <chrono>
#include <stdexcept>
#include <utility>

class VendingApplicationController::Dispenser final : public IDispenser
{
public:
    void dispense(const ProductId&) override {}
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
