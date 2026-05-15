#include "billing.h"

#include "storage_utils.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace {

bool parsePaidFlag(const std::string &value) {
    const std::string normalized = Storage::toLower(Storage::trim(value));
    return normalized == "1" || normalized == "true" || normalized == "paid" || normalized == "yes";
}

void ensureFolder(const std::string &folderName) {
#ifdef _WIN32
    _mkdir(folderName.c_str());
#else
    mkdir(folderName.c_str(), 0755);
#endif
}

} // namespace

Bill::Bill()
    : medicineCharges(0.0),
      roomCharges(0.0),
      doctorFees(0.0),
      otherCharges(0.0),
      paid(false) {}

Bill::Bill(std::string billId, std::string patientId, std::string date, std::string description,
           double medicineCharges, double roomCharges, double doctorFees,
           double otherCharges, bool paid)
    : billId(std::move(billId)),
      patientId(std::move(patientId)),
      date(std::move(date)),
      description(std::move(description)),
      medicineCharges(medicineCharges),
      roomCharges(roomCharges),
      doctorFees(doctorFees),
      otherCharges(otherCharges),
      paid(paid) {}

const std::string &Bill::getBillId() const {
    return billId;
}

const std::string &Bill::getPatientId() const {
    return patientId;
}

const std::string &Bill::getDate() const {
    return date;
}

const std::string &Bill::getDescription() const {
    return description;
}

double Bill::getMedicineCharges() const {
    return medicineCharges;
}

double Bill::getRoomCharges() const {
    return roomCharges;
}

double Bill::getDoctorFees() const {
    return doctorFees;
}

double Bill::getOtherCharges() const {
    return otherCharges;
}

double Bill::getTotalAmount() const {
    return medicineCharges + roomCharges + doctorFees + otherCharges;
}

bool Bill::isPaid() const {
    return paid;
}

void Bill::markPaid() {
    paid = true;
}

std::string Bill::serialize() const {
    return Storage::joinFields({billId,
                                patientId,
                                date,
                                description,
                                Storage::formatMoney(medicineCharges),
                                Storage::formatMoney(roomCharges),
                                Storage::formatMoney(doctorFees),
                                Storage::formatMoney(otherCharges),
                                paid ? "1" : "0"});
}

Bill Bill::fromRecord(const std::string &line, bool &ok) {
    const std::vector<std::string> fields = Storage::splitRecord(line);
    const std::string parsedBillId = fields.empty() ? "" : Storage::normalizeBillId(fields[0]);
    const std::string parsedPatientId = fields.size() > 1 ? Storage::normalizePatientId(fields[1]) : "";

    ok = fields.size() >= 6 &&
         !parsedBillId.empty() &&
         !parsedPatientId.empty();

    if (!ok) {
        return Bill();
    }

    if (fields.size() >= 9) {
        double parsedMedicine = 0.0;
        double parsedRoom = 0.0;
        double parsedDoctor = 0.0;
        double parsedOther = 0.0;

        ok = Storage::parseDouble(fields[4], parsedMedicine) &&
             Storage::parseDouble(fields[5], parsedRoom) &&
             Storage::parseDouble(fields[6], parsedDoctor) &&
             Storage::parseDouble(fields[7], parsedOther);

        if (!ok) {
            return Bill();
        }

        return Bill(parsedBillId, parsedPatientId, fields[2], fields[3],
                    parsedMedicine, parsedRoom, parsedDoctor, parsedOther,
                    parsePaidFlag(fields[8]));
    }

    // Backward compatibility for the older file format:
    // billId|patientId|date|description|amount|paid
    double oldTotalAmount = 0.0;
    ok = Storage::parseDouble(fields[4], oldTotalAmount);
    if (!ok) {
        return Bill();
    }

    return Bill(parsedBillId, parsedPatientId, fields[2], fields[3],
                0.0, 0.0, 0.0, oldTotalAmount, parsePaidFlag(fields[5]));
}

void Bill::printReceipt(const std::string &patientName) const {
    std::cout << "\n+----------------------------------------------------------------+\n";
    std::cout << "|                     CITY CARE HOSPITAL                         |\n";
    std::cout << "|                  Professional Billing Invoice                  |\n";
    std::cout << "+----------------------------------------------------------------+\n";
    std::cout << " Invoice ID     : " << billId << '\n';
    std::cout << " Generated On   : " << date << '\n';
    std::cout << " Patient        : " << patientName << '\n';
    std::cout << " Patient ID     : " << patientId << '\n';
    std::cout << " Description    : " << description << '\n';
    std::cout << "+----------------------------------------------------------------+\n";
    std::cout << std::left << std::setw(42) << " Charge Type"
              << std::right << std::setw(18) << "Amount" << '\n';
    std::cout << "+----------------------------------------------------------------+\n";
    std::cout << std::left << std::setw(42) << " Medicine Charges"
              << std::right << std::setw(18) << std::fixed << std::setprecision(2)
              << medicineCharges << '\n';
    std::cout << std::left << std::setw(42) << " Room Charges"
              << std::right << std::setw(18) << roomCharges << '\n';
    std::cout << std::left << std::setw(42) << " Doctor Fees"
              << std::right << std::setw(18) << doctorFees << '\n';
    std::cout << std::left << std::setw(42) << " Other Charges"
              << std::right << std::setw(18) << otherCharges << '\n';
    std::cout << "+----------------------------------------------------------------+\n";
    std::cout << std::left << std::setw(42) << " Total Payable"
              << std::right << std::setw(18) << getTotalAmount() << '\n';
    std::cout << std::left << std::setw(42) << " Payment Status"
              << std::right << std::setw(18) << (paid ? "Paid" : "Unpaid") << '\n';
    std::cout << "+----------------------------------------------------------------+\n";
}

bool Bill::exportReceipt(const std::string &patientName, const std::string &folderName) const {
    ensureFolder(folderName);

    const std::string fileName = folderName + "/" + Storage::safeFileName(billId) + ".txt";
    std::ofstream output(fileName);
    if (!output) {
        return false;
    }

    output << "+----------------------------------------------------------------+\n";
    output << "|                     CITY CARE HOSPITAL                         |\n";
    output << "|                  Professional Billing Invoice                  |\n";
    output << "+----------------------------------------------------------------+\n";
    output << " Invoice ID     : " << billId << '\n';
    output << " Exported On    : " << Storage::currentDateTime() << '\n';
    output << " Generated On   : " << date << '\n';
    output << " Patient        : " << patientName << '\n';
    output << " Patient ID     : " << patientId << '\n';
    output << " Description    : " << description << '\n';
    output << "+----------------------------------------------------------------+\n";
    output << std::left << std::setw(42) << " Charge Type"
           << std::right << std::setw(18) << "Amount" << '\n';
    output << "+----------------------------------------------------------------+\n";
    output << std::left << std::setw(42) << " Medicine Charges"
           << std::right << std::setw(18) << std::fixed << std::setprecision(2)
           << medicineCharges << '\n';
    output << std::left << std::setw(42) << " Room Charges"
           << std::right << std::setw(18) << roomCharges << '\n';
    output << std::left << std::setw(42) << " Doctor Fees"
           << std::right << std::setw(18) << doctorFees << '\n';
    output << std::left << std::setw(42) << " Other Charges"
           << std::right << std::setw(18) << otherCharges << '\n';
    output << "+----------------------------------------------------------------+\n";
    output << std::left << std::setw(42) << " Total Payable"
           << std::right << std::setw(18) << getTotalAmount() << '\n';
    output << std::left << std::setw(42) << " Payment Status"
           << std::right << std::setw(18) << (paid ? "Paid" : "Unpaid") << '\n';
    output << "+----------------------------------------------------------------+\n";

    return static_cast<bool>(output);
}
