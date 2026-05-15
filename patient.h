#ifndef PATIENT_H
#define PATIENT_H

#include "person.h"

#include <string>

class Patient : public Person {
private:
    std::string patientId;
    std::string bloodGroup;
    std::string disease;
    std::string address;

public:
    Patient();
    Patient(std::string patientId, std::string name, int age, std::string gender, std::string phone,
            std::string bloodGroup, std::string disease, std::string address);

    const std::string &getPatientId() const;
    const std::string &getBloodGroup() const;
    const std::string &getDisease() const;
    const std::string &getAddress() const;

    void setBloodGroup(const std::string &value);
    void setDisease(const std::string &value);
    void setAddress(const std::string &value);

    bool matchesName(const std::string &searchText) const;

    std::string serialize() const override;
    static Patient fromRecord(const std::string &line, bool &ok);

    void showDetails() const override;
    void printRow() const;
};

#endif
