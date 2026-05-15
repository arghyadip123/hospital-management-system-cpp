#ifndef DOCTOR_H
#define DOCTOR_H

#include "person.h"

#include <string>

class Doctor : public Person {
private:
    std::string doctorId;
    std::string specialization;
    std::string roomNumber;
    std::string availability;

public:
    Doctor();
    Doctor(std::string doctorId, std::string name, int age, std::string gender, std::string phone,
           std::string specialization, std::string roomNumber, std::string availability);

    const std::string &getDoctorId() const;
    const std::string &getSpecialization() const;
    const std::string &getRoomNumber() const;
    const std::string &getAvailability() const;

    void setSpecialization(const std::string &value);
    void setRoomNumber(const std::string &value);
    void setAvailability(const std::string &value);

    std::string serialize() const override;
    static Doctor fromRecord(const std::string &line, bool &ok);

    void showDetails() const override;
    void printRow() const;
};

#endif
