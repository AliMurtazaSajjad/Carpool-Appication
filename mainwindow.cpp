#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QDebug>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <windows.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadUsers();
    loadRides();

    // Set initial page
    ui->stackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    saveUsers();
    saveRides();

    // Clean up memory
    for (User* user : users) {
        delete user;
    }
    for (Ride* ride : rides) {
        delete ride;
    }

    delete ui;
}

void MainWindow::loadUsers() {
    QFile file("users.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");
            if (parts.size() >= 7) {
                QString type = parts[0];
                QString username = parts[1];
                QString password = parts[2];
                double balance = parts[3].toDouble();
                int cancelCount = parts[4].toInt();
                float rating = parts[5].toFloat();
                int ratingCount = parts[6].toInt();

                User* user = nullptr;
                if (type == "Passenger") {
                    user = new Passenger(username, password);
                } else if (type == "Captain" && parts.size() >= 9) {
                    QString vehicleType = parts[7];
                    QString vehicleClass = parts[8];
                    user = new Captain(username, password, vehicleType, vehicleClass);
                }

                if (user) {
                    user->addBalance(balance);
                    for (int i = 0; i < cancelCount; i++) user->incrementCancelCount();
                    for (int i = 0; i < ratingCount; i++) user->addRating(rating);
                    users.append(user);
                }
            }
        }
        file.close();
    }
}

void MainWindow::loadRides() {
    QFile file("rides.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");
            if (parts.size() >= 13) {
                QString captain = parts[0];
                QString passenger = parts[1];
                QString route = parts[2];
                QString depTime = parts[3];
                QString retTime = parts[4];
                QString vType = parts[5];
                QString vClass = parts[6];
                int totalSeats = parts[7].toInt();
                int occupiedSeats = parts[8].toInt();
                bool completed = parts[9] == "1";
                double fare = parts[10].toDouble();
                bool isRated = parts[11] == "1";
                QString passengersStr = parts[12];
                QStringList passengers = passengersStr.split(";");

                Ride* ride = new Ride(captain, passenger, route, depTime, retTime,
                                      vType, vClass, totalSeats, fare);
                ride->setOccupiedSeats(occupiedSeats);
                ride->setIsCompleted(completed);
                rides.append(ride);
                ride->setIsRated(isRated);
            }
        }
        file.close();
    }
}

void MainWindow::saveUsers() {
    QFile file("users.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        for (User* user : users) {
            user->saveToFile(file);
        }
        file.close();
    }
}

void MainWindow::saveRides() {
    QFile file("rides.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (Ride* ride : rides) {
            // Write all ride data in CSV format
            out << ride->getCaptain() << ","
                << ride->getPassengers().join(";") << ","  // Store all passengers separated by ;
                << ride->getRoute() << ","
                << ride->getDepartureTime() << ","
                << ride->getReturnTime() << ","
                << ride->getVehicleType() << ","
                << ride->getVehicleClass() << ","
                << ride->getTotalSeats() << ","
                << ride->getOccupiedSeats() << ","
                << (ride->getIsCompleted() ? "1" : "0") << ","
                << ride->getFare() << ","
                << (ride->getIsRated() ? "1" : "0") << "\n";
        }
        file.close();
    }
}

bool MainWindow::usernameExists(QString username) {
    for (User* user : users) {
        if (user->getUsername() == username) {
            return true;
        }
    }
    return false;
}

User* MainWindow::authenticateUser(QString username, QString password, QString userType) {
    for (User* user : users) {
        if (user->getUsername() == username &&
            user->getUserType() == userType) {
            // In a real app, we'd hash the password and compare hashes
            if (user->getPassword() == password) { // Simplified for this example
                return user;
            }
        }
    }
    return nullptr;
}

void MainWindow::on_loginButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_registerButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_passengerRegisterSelectionButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_captainRegisterSelectionButton_clicked()
{
    // Just navigate to captain registration page
    ui->stackedWidget->setCurrentIndex(4); // Index for captain registration form
}

void MainWindow::on_registerSelectionBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0); // Back to main menu
}

void MainWindow::on_passengerRegisterButton_clicked()
{
    QString username = ui->passengerRegUsername->text();
    QString password = ui->passengerRegPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username and password cannot be empty");
        return;
    }

    if (usernameExists(username)) {
        QMessageBox::warning(this, "Error", "Username already exists");
        return;
    }

    Passenger* newPassenger = new Passenger(username, password);
    users.append(newPassenger);
    saveUsers();

    QMessageBox::information(this, "Success", "Passenger account created successfully");
    ui->stackedWidget->setCurrentIndex(0);
    ui->passengerRegUsername->clear();
    ui->passengerRegPassword->clear();
}

void MainWindow::on_passengerRegisterBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->passengerRegUsername->clear();
    ui->passengerRegPassword->clear();
}

void MainWindow::on_captainRegisterButton_clicked()
{
    QString username = ui->captainRegUsername->text();
    QString password = ui->captainRegPassword->text();
    QString vehicleType = ui->vehicleTypeComboBox->currentText();
    QString vehicleClass = ui->vehicleClassComboBox->currentText();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username and password cannot be empty");
        return;
    }

    if (usernameExists(username)) {
        QMessageBox::warning(this, "Error", "Username already exists");
        return;
    }

    Captain* newCaptain = new Captain(username, password, vehicleType, vehicleClass);
    users.append(newCaptain);
    saveUsers();

    QMessageBox::information(this, "Success", "Captain account created successfully");
    ui->stackedWidget->setCurrentIndex(0);
    ui->captainRegUsername->clear();
    ui->captainRegPassword->clear();
    ui->vehicleTypeComboBox->setCurrentIndex(0);
    ui->vehicleClassComboBox->setCurrentIndex(0);
}

void MainWindow::on_captainRegisterBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->captainRegUsername->clear();
    ui->captainRegPassword->clear();
    ui->vehicleTypeComboBox->setCurrentIndex(0);
    ui->vehicleClassComboBox->setCurrentIndex(0);
}

void MainWindow::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_passengerLoginButton_clicked()
{
    ui->captainLoginUsername->clear();
    ui->captainLoginPassword->clear();

    QString username = ui->passengerLoginUsername->text();
    QString password = ui->passengerLoginPassword->text();

    currentUser = authenticateUser(username, password, "passenger");
    if (currentUser) {
        showPassengerDashboard();
        updatePassengerBalanceDisplay();
        updatePassengerRatingDisplay();
        ui->passengerLoginUsername->clear();
        ui->passengerLoginPassword->clear();
    } else {
        QMessageBox::warning(this, "Error", "Invalid username or password");
        ui->passengerLoginPassword->clear();
    }
}

void MainWindow::on_captainLoginButton_clicked()
{
    ui->passengerLoginUsername->clear();
    ui->passengerLoginPassword->clear();

    QString username = ui->captainLoginUsername->text();
    QString password = ui->captainLoginPassword->text();

    currentUser = authenticateUser(username, password, "captain");
    if (currentUser) {
        showCaptainDashboard();
        ui->captainLoginUsername->clear();
        ui->captainLoginPassword->clear();
    } else {
        QMessageBox::warning(this, "Error", "Invalid username or password");
        ui->captainLoginPassword->clear();
    }
}

void MainWindow::on_loginBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->passengerLoginUsername->clear();
    ui->passengerLoginPassword->clear();
    ui->captainLoginUsername->clear();
    ui->captainLoginPassword->clear();
}

void MainWindow::showPassengerDashboard() {
    ui->passengerWelcomeLabel->setText("Welcome, " + currentUser->getUsername());
    updatePassengerBalanceDisplay();
    ui->stackedWidget->setCurrentIndex(5);
}

void MainWindow::showCaptainDashboard() {
    ui->captainWelcomeLabel->setText("Welcome, " + currentUser->getUsername());
    updateCaptainBalanceDisplay();
    ui->stackedWidget->setCurrentIndex(6);
    displayCaptainRides();
    // Show captain's rating
    if (currentUser && currentUser->getUserType() == "captain") {
        QString ratingText = QString("Your Rating: ★%1 (%2 ratings)")
                                 .arg(currentUser->getAverageRating(), 0, 'f', 1)
                                 .arg(currentUser->getRatingCount());
        ui->captainRatingLabel->setText(ratingText);
    }
}

void MainWindow::updateCaptainBalanceDisplay() {
    ui->captainLiveBalanceLabel->setText("Balance Rs " + QString::number(currentUser->getBalance(), 'f', 2));
}

void MainWindow::on_passengerDashboardBackButton_clicked()
{
    currentUser = nullptr;
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_captainDashboardBackButton_clicked()
{
    currentUser = nullptr;
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_addBalanceButton_clicked()
{
    bool ok;
    double amount = QInputDialog::getDouble(this,
                                            "Add Balance",
                                            "Enter amount to add:",
                                            0,    // default value
                                            0,    // min value
                                            10000, // max value
                                            2,    // decimals
                                            &ok);

    if (ok && amount > 0) {
        currentUser->addBalance(amount);
        updatePassengerBalanceDisplay();
        saveUsers();
        QMessageBox::information(this, "Success",
                                 QString("Added Rs %1 to your balance").arg(amount));
    }
}

void MainWindow::updatePassengerBalanceDisplay()
{
    if (currentUser && currentUser->getUserType() == "passenger") {
        QString balanceText = QString("Balance: Rs %1")
                                  .arg(currentUser->getBalance(), 0, 'f', 2);

        ui->passengerBalanceLabel->setText(balanceText);

        qApp->processEvents();
    }
}

void MainWindow::on_addCaptainBalanceButton_clicked()
{
    bool ok;
    double amount = QInputDialog::getDouble(this, "Add Balance", "Enter amount to add:", 0, 0, 10000, 2, &ok);
    if (ok) {
        currentUser->addBalance(amount);
        updateCaptainBalanceDisplay();
        saveUsers();
    }
}

void MainWindow::on_createRideSelectionButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(7);
}

void MainWindow::on_createRideButton_clicked()
{
    QString route = ui->rideRouteEdit->text();
    QString depTime = ui->departureTimeEdit->text();
    QString retTime = ui->returnTimeEdit->text();
    int seats = ui->seatsSpinBox->value();
    double fare = ui->fareSpinBox->value();

    if (route.isEmpty() || depTime.isEmpty()) {
        QMessageBox::warning(this, "Error", "Route and departure time are required");
        return;
    }

    Captain* captain = dynamic_cast<Captain*>(currentUser);
    if (!captain) return;

    Ride* newRide = new Ride(currentUser->getUsername(), "", route, depTime, retTime,
                             captain->getVehicleType(), captain->getVehicleClass(), seats, fare);
    rides.append(newRide);
    saveRides();

    Beep(800, 200);
    Beep(1000, 200);
    QMessageBox::information(this, "Success", "Ride created successfully");
    ui->stackedWidget->setCurrentIndex(6);
    ui->rideRouteEdit->clear();
    ui->departureTimeEdit->clear();
    ui->returnTimeEdit->clear();
    ui->seatsSpinBox->setValue(0);
    ui->fareSpinBox->setValue(0.0);
}

void MainWindow::on_createRideBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(6);
}

void MainWindow::on_captainRidesBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(6);
}

void MainWindow::displayAvailableRides() {
    ui->availableRidesList->clear();

    for (Ride* ride : rides) {

        if (ride->getIsCompleted() || ride->isFull()) {
            continue;
        }

        // Skip if current user already booked this ride
        if (ride->getPassengers().contains(currentUser->getUsername())) {
            continue;
        }

        if (!ride->getIsCompleted() && ride->getOccupiedSeats() < ride->getTotalSeats()) {
            QString rideInfo = QString("Route: %1 | Departure: %2 | Vehicle: %3 %4 | Seats: %5/%6 | Fare: Rs %7")
            .arg(ride->getRoute())
                .arg(ride->getDepartureTime())
                .arg(ride->getVehicleType())
                .arg(ride->getVehicleClass())
                .arg(ride->getOccupiedSeats())
                .arg(ride->getTotalSeats())
                .arg(ride->getFare());

            QListWidgetItem* item = new QListWidgetItem(rideInfo);
            item->setData(Qt::UserRole, QVariant::fromValue(ride));
            ui->availableRidesList->addItem(item);
        }
    }
}

void MainWindow::on_bookRideButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(8);
    displayAvailableRides();
    qDebug() << "Book Ride button clicked - showing available rides";
}

void MainWindow::on_viewMyRidesButton_clicked()
{
    if (!currentUser) {
        QMessageBox::warning(this, "Error", "No user logged in");
        return;
    }

    ui->myRidesList->clear();

    for (Ride* ride : rides) {
        if (ride->getPassenger() == currentUser->getUsername() && !ride->getIsCompleted()) {
            QString rideInfo = QString("Route: %1 | Captain: %2 | Departure: %3 | Status: %4")
            .arg(ride->getRoute())
                .arg(ride->getCaptain())
                .arg(ride->getDepartureTime())
                .arg(ride->getOccupiedSeats() > 0 ? "Booked" : "Pending");

            QListWidgetItem* item = new QListWidgetItem(rideInfo);
            item->setData(Qt::UserRole, QVariant::fromValue(ride));
            ui->myRidesList->addItem(item);
        }
    }
    if (ui->myRidesList->count() > 0) {
        ui->myRidesList->setCurrentRow(0);
    }

    ui->stackedWidget->setCurrentIndex(10);
    qDebug() << "Showing" << ui->myRidesList->count() << "active rides";
}

void MainWindow::on_myRidesBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}

void MainWindow::on_viewRidesButton_clicked()
{
    ui->captainRidesList->clear();

    for (Ride* ride : rides) {
        if (ride->getCaptain() == currentUser->getUsername() && !ride->getIsCompleted()) {
            QString status = ride->getPassenger().isEmpty() ? "Available" : "Booked by " + ride->getPassenger();
            QString rideInfo = QString("Route: %1 | %2 | Seats: %3/%4 | Fare: Rs %5")
                                   .arg(ride->getRoute())
                                   .arg(status)
                                   .arg(ride->getOccupiedSeats())
                                   .arg(ride->getTotalSeats())
                                   .arg(ride->getFare());

            QListWidgetItem* item = new QListWidgetItem(rideInfo);
            item->setData(Qt::UserRole, QVariant::fromValue(ride));
            ui->captainRidesList->addItem(item);
        }
    }
    displayCaptainRides();
    ui->stackedWidget->setCurrentIndex(9);
}

void MainWindow::on_passengerCancelRideButton_clicked()
{
    // 1. Verify passenger is logged in
    if (!currentUser || currentUser->getUserType() != "passenger") {
        QMessageBox::warning(this, "Error", "No passenger logged in");
        return;
    }

    // 2. Find all active rides for this passenger
    QList<Ride*> passengerRides;
    for (Ride* ride : rides) {
        if (ride->getPassenger() == currentUser->getUsername() && !ride->getIsCompleted()) {
            passengerRides.append(ride);
        }
    }

    // 3. Check if passenger has any active rides
    if (passengerRides.isEmpty()) {
        QMessageBox::information(this, "No Rides", "You don't have any active rides to cancel");
        return;
    }

    // 4. For simplicity, let's cancel the first found ride
    // (You can modify this to cancel most recent or let user choose)
    Ride* rideToCancel = passengerRides.first();

    // 5. Show confirmation dialog with ride details
    QString confirmMsg = QString("Are you sure you want to cancel this ride?\n\n"
                                 "Route: %1\n"
                                 "Captain: %2\n"
                                 "Time: %3\n"
                                 "Fare: Rs %4")
                             .arg(rideToCancel->getRoute())
                             .arg(rideToCancel->getCaptain())
                             .arg(rideToCancel->getDepartureTime())
                             .arg(rideToCancel->getFare(), 0, 'f', 2);

    if (QMessageBox::question(this, "Confirm Cancellation", confirmMsg,
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    // 6. Calculate refund amount (with penalty if applicable)
    double refundAmount = rideToCancel->getFare();
    double penalty = 0.0;

    if (currentUser->getCancelCount() >= 2) {
        penalty = 50.0; // Rs 50 penalty after 2 cancellations
        refundAmount -= penalty;
    }

    // 7. Update passenger balance
    currentUser->addBalance(refundAmount);
    currentUser->incrementCancelCount();

    // 8. Update ride status
    rideToCancel->setPassenger("");
    rideToCancel->setOccupiedSeats(rideToCancel->getOccupiedSeats() - 1);

    // 9. Update captain's balance
    for (User* user : users) {
        if (user->getUsername() == rideToCancel->getCaptain()) {
            user->deductBalance(refundAmount);
            break;
        }
    }

    // 10. Save all changes
    saveUsers();
    saveRides();

    // 11. Show success message
    QString resultMsg = QString("Ride cancelled successfully!\n\n"
                                "Refunded: Rs %1")
                            .arg(refundAmount, 0, 'f', 2);

    if (penalty > 0) {
        resultMsg += QString("\nCancellation fee: Rs %1").arg(penalty, 0, 'f', 2);
    }

    QMessageBox::information(this, "Cancellation Complete", resultMsg);

    // 12. Update UI
    updatePassengerBalanceDisplay();
}

void MainWindow::on_availableRidesBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);

}

void MainWindow::on_availableRidesList_itemDoubleClicked(QListWidgetItem *item)
{
    Ride* ride = item->data(Qt::UserRole).value<Ride*>();
    if (!ride) return;

    // Check if passenger already has 2 active rides
    int activeRides = 0;
    for (Ride* r : rides) {
        if (r->getPassenger() == currentUser->getUsername() && !r->getIsCompleted()) {
            activeRides++;
        }
    }

    if (ride->getPassengers().contains(currentUser->getUsername())) {
        QMessageBox::warning(this, "Error", "You already booked this ride");
        return;
    }

    if (activeRides >= 2) {
        QMessageBox::warning(this, "Error", "You can only have 2 active rides at a time");
        return;
    }
    if (ride->getOccupiedSeats() >= ride->getTotalSeats()) {
        QMessageBox::warning(this, "Error", "No seats available on this ride");
        return;
    }
    // Check balance
    double totalFare = ride->getFare();
    if (currentUser->getBalance() < totalFare) {
        QMessageBox::warning(this, "Error", "Insufficient balance");
        return;
    }

    // Deduct fare with 5% platform fee
    double platformFee = totalFare * 0.05;
    double captainEarning = totalFare - platformFee;

    currentUser->deductBalance(totalFare);

    // Find captain and add balance
    for (User* user : users) {
        if (user->getUsername() == ride->getCaptain()) {
            user->addBalance(captainEarning);
            break;
        }
    }

    // Update ride
    ride->setOccupiedSeats(ride->getOccupiedSeats() + 1);

    if (ride->getPassenger().isEmpty()) {
        ride->setPassenger(currentUser->getUsername());
    }
    ride->addPassenger(currentUser->getUsername());

    saveUsers();
    saveRides();
    updatePassengerBalanceDisplay();
    displayAvailableRides();
    Beep(500, 150);
    Beep(600, 150);
    QMessageBox::information(this, "Success",
                             QString("Booked seat on %1's ride\nSeats: %2/%3")
                                 .arg(ride->getCaptain())
                                 .arg(ride->getOccupiedSeats())
                                 .arg(ride->getTotalSeats()));
    ui->stackedWidget->setCurrentIndex(5);
}

void MainWindow::on_completeRideButton_clicked()
{
    QListWidgetItem* item = ui->captainRidesList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Error", "No ride selected");
        return;
    }

    Ride* ride = item->data(Qt::UserRole).value<Ride*>();
    if (!ride) {
        QMessageBox::critical(this, "Error", "Invalid ride data");
        return;
    }

    // Mark as completed but don't delete
    ride->setIsCompleted(true);

    // Save changes
    saveRides();

    // Refresh displays (will hide completed rides from active lists)
    displayCaptainRides();
    displayAvailableRides();

    QMessageBox::information(this, "Completed",
                             "Ride marked as completed. Passengers can now rate this ride.");
}

void MainWindow::on_cancelRideButton_clicked()
{
    QListWidgetItem* item = ui->captainRidesList->currentItem();
    if (!item) return;

    Ride* ride = item->data(Qt::UserRole).value<Ride*>();
    if (!ride) return;

    if (ride->getPassenger().isEmpty()) {
        // Captain is canceling an available ride
        rides.removeOne(ride);
        delete ride;
    } else {
        // Captain is canceling a booked ride
        if (currentUser->getCancelCount() >= 2) {
            // Apply penalty after 2 cancellations
            currentUser->deductBalance(50);
        }
        currentUser->incrementCancelCount();

        // Refund passenger (simplified - in reality would need proper transaction handling)
        for (User* user : users) {
            if (user->getUsername() == ride->getPassenger()) {
                user->addBalance(ride->getFare());
                break;
            }
        }

        ride->setPassenger("");
        ride->setOccupiedSeats(0);
    }

    saveUsers();
    saveRides();

    if (currentUser->getUserType() == "passenger") {
        updatePassengerBalanceDisplay();
    } else {
        updateCaptainBalanceDisplay();
    }

    QMessageBox::information(this, "Success", "Ride canceled successfully");

    if (currentUser->getUserType() == "passenger") {
        ui->stackedWidget->setCurrentIndex(3);
    } else {
        on_viewRidesButton_clicked();
    }
}

void MainWindow::on_rateUserButton_clicked()
{
    // 1. Get currently selected ride
    QListWidgetItem* item = ui->captainRidesList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Error", "Please select a ride first");
        return;
    }

    // 2. Get the Ride object
    Ride* ride = item->data(Qt::UserRole).value<Ride*>();
    if (!ride || ride->getPassenger().isEmpty()) {
        QMessageBox::warning(this, "Error", "No passenger to rate for this ride");
        return;
    }

    // 3. Find the passenger in users list
    User* passenger = nullptr;
    for (User* user : users) {
        if (user->getUsername() == ride->getPassenger()) {
            passenger = user;
            break;
        }
    }

    if (!passenger) {
        QMessageBox::critical(this, "Error", "Passenger not found");
        return;
    }

    // 4. Show rating dialog
    bool ok;
    int rating = QInputDialog::getInt(this, "Rate Passenger",
                                      QString("Rate passenger %1 (1-5 stars):")
                                          .arg(passenger->getUsername()),
                                      3, 1, 5, 1, &ok);

    if (!ok) return; // User cancelled

    // 5. Add the rating
    passenger->addRating(rating);

    // 6. Save changes
    saveUsers();

    updatePassengerRatingDisplay();
    // 7. Show confirmation
    QMessageBox::information(this, "Rating Submitted",
                             QString("You rated %1 with %2 stars")
                                 .arg(passenger->getUsername())
                                 .arg(rating));
}
void MainWindow::displayCaptainRides()
{
    ui->captainRidesList->clear();

    for (Ride* ride : rides) {
        if (ride->getCaptain() == currentUser->getUsername() && !ride->getIsCompleted() ) {
            // Find passenger to get rating
            float passengerRating = 0;
            for (User* user : users) {
                if (user->getUsername() == ride->getPassenger()) {
                    passengerRating = user->getAverageRating();
                    break;
                }
            }

            QString status = ride->getPassenger().isEmpty()
                                 ? "Available"
                                 : QString("Booked by %1 (Rating: %2)")
                                       .arg(ride->getPassenger())
                                       .arg(passengerRating, 0, 'f', 1);

            QString rideInfo = QString("Route: %1 | %2 | Seats: %3/%4")
                                   .arg(ride->getRoute())
                                   .arg(status)
                                   .arg(ride->getOccupiedSeats())
                                   .arg(ride->getTotalSeats());

            QListWidgetItem* item = new QListWidgetItem(rideInfo);
            item->setData(Qt::UserRole, QVariant::fromValue(ride));
            ui->captainRidesList->addItem(item);
        }
    }
}

void MainWindow::updatePassengerRatingDisplay()
{
    if (currentUser && currentUser->getUserType() == "passenger") {
        QString ratingText = QString("Rating: ★%1")
                                 .arg(currentUser->getAverageRating(), 0, 'f', 1);
        ui->passengerRatingLabel->setText(ratingText);
    }
}

void MainWindow::on_rateCaptainButton_clicked()
{
    // Find completed rides that haven't been rated yet
    QList<Ride*> rateableRides;
    for (Ride* ride : rides) {
        if (ride->getPassenger() == currentUser->getUsername() &&
            ride->getIsCompleted() && !ride->getIsRated()) {
            rateableRides.append(ride);
        }
    }

    if (rateableRides.isEmpty()) {
        QMessageBox::information(this, "No Rides", "No completed rides available to rate");
        return;
    }

    // For simplicity, let's rate the most recent completed ride
    Ride* rideToRate = nullptr;
    QDateTime latestTime;
    for (Ride* ride : rateableRides) {
        QDateTime rideTime = QDateTime::fromString(ride->getDepartureTime(), "yyyy-MM-dd hh:mm");
        if (!rideToRate || rideTime > latestTime) {
            rideToRate = ride;
            latestTime = rideTime;
        }
    }

    if (rideToRate) {
        showCaptainRatingDialog(rideToRate);
    }
}

void MainWindow::showCaptainRatingDialog(Ride* ride)
{
    QDialog ratingDialog(this);
    ratingDialog.setWindowTitle("Rate Your Captain");

    // Use pointers for layouts
    QVBoxLayout* mainLayout = new QVBoxLayout(&ratingDialog);

    QLabel* caption = new QLabel(
        QString("How was your ride with Captain %1?").arg(ride->getCaptain()),
        &ratingDialog
        );
    mainLayout->addWidget(caption);

    QWidget* starWidget = new QWidget(&ratingDialog);
    QHBoxLayout* starLayout = new QHBoxLayout(starWidget);

    for (int i = 1; i <= 5; i++) {
        QPushButton* star = new QPushButton("☆", starWidget);
        star->setProperty("rating", i);
        star->setStyleSheet("font-size: 24px; border: none;");
        connect(star, &QPushButton::clicked, [this, &ratingDialog, ride, i]() {
            processCaptainRating(ride, i);
            ratingDialog.accept();
        });
        starLayout->addWidget(star);
    }

    mainLayout->addWidget(starWidget);
    ratingDialog.setLayout(mainLayout);
    ratingDialog.exec();
}

void MainWindow::processCaptainRating(Ride* ride, int rating)
{
    if (!ride || !currentUser) return;

    // Find captain user
    User* captain = nullptr;
    for (User* user : users) {
        if (user->getUsername() == ride->getCaptain()) {
            captain = user;
            break;
        }
    }

    if (!captain) {
        QMessageBox::critical(this, "Error", "Captain not found");
        return;
    }

    // Add rating
    captain->addRating(rating);
    ride->setIsRated(true);

    // Save changes
    saveUsers();
    saveRides();

    // Update captain's rating display if we're on their dashboard
    if (currentUser->getUsername() == captain->getUsername()) {
        updateCaptainRatingDisplay();
    }

    QMessageBox::information(this, "Thank You",
                             QString("You rated Captain %1 with %2 stars")
                                 .arg(captain->getUsername())
                                 .arg(rating));
}

void MainWindow::updateCaptainRatingDisplay()
{
    if (currentUser && currentUser->getUserType() == "captain") {
        QString ratingText = QString("Your Rating: ★%1 (%2 ratings)")
                                 .arg(currentUser->getAverageRating(), 0, 'f', 1)
                                 .arg(currentUser->getRatingCount());
        ui->captainRatingLabel->setText(ratingText);
    }
}
