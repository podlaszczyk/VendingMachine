#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include <memory>

class ICardReader;
class VendingMachine;
class TransactionRepository;

class VendingApplicationController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int pendingTransactions READ pendingTransactions NOTIFY pendingTransactionsChanged)
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)

public:
    explicit VendingApplicationController(QObject* parent = nullptr);
    VendingApplicationController(ICardReader& cardReader, VendingMachine& machine, QObject* parent = nullptr);
    ~VendingApplicationController() override;

    [[nodiscard]] QString state() const;
    [[nodiscard]] int pendingTransactions() const;
    [[nodiscard]] bool online() const;

    Q_INVOKABLE void simulateCardTap();
    Q_INVOKABLE void selectProduct(const QString& productId);
    Q_INVOKABLE void simulateDispenseFailure();
    Q_INVOKABLE void reset();

signals:
    void stateChanged();
    void pendingTransactionsChanged();
    void onlineChanged();
    void errorOccurred(const QString& message);

private:
    class Dispenser;
    class SyncRunner;

    void refreshState();
    void refreshPendingTransactions();
    void finishDispensing(bool success);
    void connectCardReader(ICardReader& cardReader);
    void setupSelectionTimeout();
    void startSelectionTimeout();
    void stopSelectionTimeout();
    void startSynchronization();
    void stopSynchronization();
    void refreshSynchronizationStatus();

    std::unique_ptr<Dispenser> dispenser;
    std::unique_ptr<TransactionRepository> repository;
    std::unique_ptr<VendingMachine> ownedMachine;
    VendingMachine* machine{nullptr};
    ICardReader* cardReader{nullptr};
    QString lastState;
    QString activeTransactionId;
    int pendingCount{0};
    bool isOnline{false};
    QTimer selectionTimeoutTimer;
    QTimer synchronizationStatusTimer;
    SyncRunner* syncRunner{nullptr};
    std::unique_ptr<class QThread> syncThread;
};
