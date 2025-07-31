#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qlistwidget.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class User {
protected:
    QString username;
    QString password;
    QString userType;
    double balance;
    int cancelCount;
    float rating;
    int ratingCount;
    float totalRating = 0;

public:
    User(QString uname, QString pwd, QString type) : username(uname), password(pwd), userType(type), balance(0), cancelCount(0), rating(0), ratingCount(0) {}
    virtual ~User() {}

    QString getUsername() const { return username; }
    QString getUserType() const { return userType; }
    QString getPassword() const { return password; }
    double getBalance() const { return balance; }
    int getCancelCount() const { return cancelCount; }
    float getRating() const { return ratingCount > 0 ? rating / ratingCount : 0; }
    float getAverageRating() const {
        return ratingCount > 0 ? totalRating / ratingCount : 0;
    }
    int getRatingCount() const {
        return ratingCount;
    }

    void addBalance(double amount) { balance += amount; }
    void deductBalance(double amount) { balance -= amount; }
    void incrementCancelCount() { cancelCount++; }
    void addRating(int stars) {
        totalRating += stars;
        ratingCount++;
    }
    virtual void saveToFile(QFile &file) = 0;
};

class Passenger : public User {
public:
    Passenger(QString uname, QString pwd) : User(uname, pwd, "passenger") {}

    void saveToFile(QFile &file) override {
        QTextStream out(&file);
        out << "Passenger," << username << "," << password << "," << balance << ","
            << cancelCount << "," << rating << "," << ratingCount << "\n";
    }
};

class Captain : public User {
    QString vehicleType;
    QString vehicleClass;

public:
    Captain(QString uname, QString pwd, QString vType, QString vClass)
        : User(uname, pwd, "captain"), vehicleType(vType), vehicleClass(vClass) {}

    QString getVehicleType() const { return vehicleType; }
    QString getVehicleClass() const { return vehicleClass; }

    void saveToFile(QFile &file) override {
        QTextStream out(&file);
        out << "Captain," << username << "," << password << "," << balance << ","
            << cancelCount << "," << rating << "," << ratingCount << ","
            << vehicleType << "," << vehicleClass << "\n";
    }
};

class Ride {
    QString captainUsername;
    QString passengerUsername;
    QString route;
    QString departureTime;
    QString returnTime;
    QString vehicleType;
    QString vehicleClass;
    QStringList passengers;
    int totalSeats;
    int occupiedSeats;
    bool isCompleted;
    double fare;
    bool isRated = false;

public:
    Ride(QString capUser, QString passUser, QString rt, QString depTime, QString retTime,
         QString vType, QString vClass, int seats, double fr)
        : captainUsername(capUser), passengerUsername(passUser), route(rt),
        departureTime(depTime), returnTime(retTime), vehicleType(vType),
        vehicleClass(vClass), totalSeats(seats), occupiedSeats(0), isCompleted(false), fare(fr) {}

    bool addPassenger(const QString &username) {
        if (passengers.size() < totalSeats && !passengers.contains(username)) {
            passengers.append(username);
            return true;
        }
        return false;
    }

    QString getPassengersString() const {
        return passengers.join(", ");
    }

    void saveToFile(QFile &file) {
        QTextStream out(&file);
        out << captainUsername << "," << passengerUsername << ","
            << route << "," << departureTime << ","
            << returnTime << "," << vehicleType << ","
            << vehicleClass << "," << totalSeats << ","
            << occupiedSeats << "," << (isCompleted ? "1" : "0") << ","
            << fare << "," << (isRated ? "1" : "0") << "\n";
    }

    // Getters
    QString getCaptain() const { return captainUsername; }
    QString getPassenger() const { return passengerUsername; }
    QString getRoute() const { return route; }
    QString getDepartureTime() const { return departureTime; }
    QString getReturnTime() const { return returnTime; }
    QString getVehicleType() const { return vehicleType; }
    QStringList getPassengers() const { return passengers; }
    QString getVehicleClass() const { return vehicleClass; }
    int getTotalSeats() const { return totalSeats; }
    int getOccupiedSeats() const { return occupiedSeats; }
    int getAvailableSeats() const { return totalSeats - passengers.size(); }
    bool getIsCompleted() const { return isCompleted; }
    double getFare() const { return fare; }
    bool getIsRated() const { return isRated; }
    bool isFull() const { return passengers.size() >= totalSeats; }

    // Setters
    void setPassenger(const QString &passenger) { passengerUsername = passenger; }
    void setOccupiedSeats(int seats) { occupiedSeats = seats; }
    void setIsCompleted(bool completed) { isCompleted = completed; }
    void setIsRated(bool rated) { isRated = rated; }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_passengerRegisterButton_clicked();
    void on_captainRegisterButton_clicked();
    void on_backButton_clicked();
    void on_passengerLoginButton_clicked();
    void on_captainLoginButton_clicked();
    void on_passengerDashboardBackButton_clicked();
    void on_captainDashboardBackButton_clicked();
    void on_addCaptainBalanceButton_clicked();
    void on_addBalanceButton_clicked();
    void on_createRideSelectionButton_clicked();
    void on_createRideButton_clicked();
    void on_bookRideButton_clicked();
    void on_viewRidesButton_clicked();
    void on_completeRideButton_clicked();
    void on_cancelRideButton_clicked();
    void on_rateUserButton_clicked();
    void on_availableRidesList_itemDoubleClicked(QListWidgetItem *item);
    void on_passengerRegisterSelectionButton_clicked();
    void on_captainRegisterSelectionButton_clicked();
    void on_registerSelectionBackButton_clicked();
    void on_captainRegisterBackButton_clicked();
    void on_passengerRegisterBackButton_clicked();
    void on_loginBackButton_clicked();
    void on_createRideBackButton_clicked();
    void on_captainRidesBackButton_clicked();
    void on_viewMyRidesButton_clicked();
    void on_myRidesBackButton_clicked();
    void on_passengerCancelRideButton_clicked();
    void on_rateCaptainButton_clicked();
    void showCaptainRatingDialog(Ride* completedRide);
    void processCaptainRating(Ride* ride, int rating);
    void on_availableRidesBackButton_clicked();

private:
    Ui::MainWindow *ui;
    User* currentUser;
    Ride* currentRide;

    void loadUsers();
    void loadRides();
    void saveUsers();
    void saveRides();
    void showPassengerDashboard();
    void showCaptainDashboard();
    void updatePassengerBalanceDisplay();
    void updateCaptainBalanceDisplay();
    void displayAvailableRides();
    void displayCaptainRides();
    void updatePassengerRatingDisplay();
    void updateCaptainRatingDisplay();

    User* authenticateUser(QString username, QString password, QString userType);
    bool usernameExists(QString username);

    QList<User*> users;
    QList<Ride*> rides;
};

#endif // MAINWINDOW_H
