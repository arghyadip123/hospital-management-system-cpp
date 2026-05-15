#ifndef BILLING_H
#define BILLING_H

#include <string>

class Bill {
private:
    std::string billId;
    std::string patientId;
    std::string date;
    std::string description;
    double medicineCharges;
    double roomCharges;
    double doctorFees;
    double otherCharges;
    bool paid;

public:
    Bill();
    Bill(std::string billId, std::string patientId, std::string date, std::string description,
         double medicineCharges, double roomCharges, double doctorFees,
         double otherCharges, bool paid);

    const std::string &getBillId() const;
    const std::string &getPatientId() const;
    const std::string &getDate() const;
    const std::string &getDescription() const;
    double getMedicineCharges() const;
    double getRoomCharges() const;
    double getDoctorFees() const;
    double getOtherCharges() const;
    double getTotalAmount() const;
    bool isPaid() const;

    void markPaid();

    std::string serialize() const;
    static Bill fromRecord(const std::string &line, bool &ok);

    void printReceipt(const std::string &patientName) const;
    bool exportReceipt(const std::string &patientName, const std::string &folderName) const;
};

#endif
