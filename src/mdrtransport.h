#pragma once

#include <QObject>
#include <QTimer>

struct MDRConnectionLinux;
struct MDRHeadphones;

class MdrTransport final : public QObject {
    Q_OBJECT
public:
    explicit MdrTransport(QObject *parent = nullptr);
    ~MdrTransport() override;
    void connectTo(const QString &address);
    void disconnect();
    bool ready() const { return m_ready; }
    QString status() const { return m_status; }
signals:
    void stateChanged();
    void batteriesChanged(int left, int right, int caseLevel);
private slots:
    void poll();
private:
    void fail(const QString &message);
    void updateBatteries();
    MDRConnectionLinux *m_linux = nullptr;
    MDRHeadphones *m_headphones = nullptr;
    QTimer m_timer;
    QString m_address;
    QString m_status = "MDR transport idle";
    bool m_connecting = false;
    bool m_ready = false;
};
