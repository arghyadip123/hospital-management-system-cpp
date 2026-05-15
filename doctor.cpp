#include "doctor.h"

#include "storage_utils.h"

#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

Doctor::Doctor() = default;

Doctor::Doctor(std::string doctorId, std::string name, int age, std::string gender, std::string phone,
               std::string specialization, std::string roomNumber, std::string availability)
    : Person(0, std::move(name), age, std::move(gender), std::move(phone)),
      doctorId(std::move(doctorId)),
      specialization(std::move(specialization)),
      roomNumber(std::move(roomNumber)),
      availability(std::move(availability)) {}

const std::string &Doctor::getDoctorId() const {
    return doctorId;
}

const std::string &Doctor::getSpecialization() const {
    return specialization;
}

const std::string &Doctor::getRoomNumber() const {
    return roomNumber;
}

const std::string &Doctor::getAvailability() const {
    return availability;
}

void Doctor::setSpecialization(const std::string &value) {
    specialization = value;
}

void Doctor::setRoomNumber(const std::string &value) {
    roomNumber = value;
}

void Doctor::setAvailability(const std::string &value) {
    availability = value;
}

std::string Doctor::serialize() const {
    return Storage::joinFields({doctorId, name, std::to_string(age), gender, phone,
                                specialization, roomNumber, availability});
}

Doctor Doctor::fromRecord(const std::string &line, bool &ok) {
    const std::vector<std::string> fields = Storage::splitRecord(line);
    const std::string parsedDoctorId = fields.empty() ? "" : Storage::normalizeDoctorId(fields[0]);
    int parsedAge = 0;

    ok = fields.size() >= 8 &&
         !parsedDoctorId.empty() &&
         Storage::parseInt(fields[2], parsedAge);

    if (!ok) {
        return Doctor();
    }

    return Doctor(parsedDoctorId, fields[1], parsedAge, fields[3], fields[4],
                  fields[5], fields[6], fields[7]);
}

void Doctor::showDetails() const {
    std::cout << "\nDoctor ID      : " << doctorId
              << "\nName           : " << name
              << "\nAge            : " << age
              << "\nGender         : " << gender
              << "\nPhone          : " << phone
              << "\nSpecialization : " << specialization
              << "\nRoom Number    : " << roomNumber
              << "\nAvailability   : " << availability << '\n';
}

void Doctor::printRow() const {
    std::cout << std::left << std::setw(12) << doctorId
              << std::setw(24) << name.substr(0, 23)
              << std::setw(24) << specialization.substr(0, 23)
              << std::setw(12) << roomNumber.substr(0, 11)
              << std::setw(18) << availability.substr(0, 17)
              << std::setw(16) << phone.substr(0, 15) << '\n';
}
