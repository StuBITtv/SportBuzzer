#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-pass-by-value"

#include "src/GUI/MainMenu.hpp"
#include "src/Wireless/Connection.hpp"

#define BUTTON 2
#define TIMEOUT 2000

volatile unsigned long buzzerTime = 0;

void setup() {
    Serial.begin(9600);

    Connection::hc12.start(B9600);

    pinMode(BUTTON, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BUTTON), []() {
        buzzerTime = millis();
    }, RISING);

    Connection::testModule();
    Connection::setup();

    Connection::drawConnectionStatus(TransmissionStatus::timeout);
}

void loop() {
    static GUITask *task = new MainMenu();
    static GUITask *prevTask = nullptr;

    // region update battery status
    // TODO
    // endregion

    static Transmissions transmissions(Connection::hc12);
    transmissions.poll();

    // region gui task handler
    auto newTask = task->update(transmissions, buzzerTime, task != prevTask);
    if (newTask != nullptr) {
        prevTask = task;
        task = newTask;

        if (prevTask != task) {
            delete prevTask;
        }
    } else {
        Serial.println("ERROR: No next task provided after finishing the current task.");
        Serial.println();
    }
    // endregion

    Connection::handlePingSignals(transmissions);

    // region check connection
    if (millis() - transmissions.getPingTime() > TIMEOUT + 100) {
        transmissions.sendPing(TIMEOUT);
    }

    Connection::drawConnectionStatus(transmissions.getPingStatus());
    // endregion
}

#pragma clang diagnostic pop
