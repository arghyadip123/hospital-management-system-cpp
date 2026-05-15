#include "patient.h"

#include "storage_utils.h"

#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

Patient::Patient() = default;

Patient::Patient(std::string patientId, std::string name, int age, std::string gender, std::string phone,
                 std::string bloodGroup, std::string disease, std::string address)
    : Person(0, std::move(name), age, std::move(gender), std::move(phone)),
      patientId(std::move(patientId)),
      bloodGroup(std::move(bloodGroup)),
      disease(std::move(disease)),
      address(std::move(address)) {}

const std::string &Patient::getPatientId() const {
    return patientId;
}

const std::string &Patient::getBloodGroup() const {
    return bloodGroup;
}

const std::string &Patient::getDisease() const {
    return disease;
}

const std::string &Patient::getAddress() const {
    return address;
}

void Patient::setBloodGroup(const std::string &value) {
    bloodGroup = value;
}

void Patient::setDisease(const std::string &value) {
    disease = value;
}

void Patient::setAddress(const std::string &value) {
    address = value;
}

bool Patient::matchesName(const std::string &searchText) const {
    return Storage::toLower(name).find(Storage::toLower(Storage::trim(searchText))) != std::string::npos;
}

std::string Patient::serialize() const {
    return Storage::joinFields({patientId, name, std::to_string(age), gender, phone,
                                bloodGroup, disease, address});
}

Patient Patient::fromRecord(const std::string &line, bool &ok) {
    const std::vector<std::string> fields = Storage::splitRecord(line);
    const std::string parsedPatientId = Storage::normalizePatientId(fields.empty() ? "" : fields[0]);
    int parsedAge = 0;

    ok = fields.size() >= 8 &&
         !parsedPatientId.empty() &&
         Storage::parseInt(fields[2], parsedAge);

    if (!ok) {
        return Patient();
    }

    return Patient(parsedPatientId, fields[1], parsedAge, fields[3], fields[4],
                   fields[5], fields[6], fields[7]);
}

void Patient::showDetails() const {
    std::cout << "\nPatient ID   : " << patientId
              << "\nName         : " << name
              << "\nAge          : " << age
              << "\nGender       : " << gender
              << "\nPhone        : " << phone
              << "\nBlood Group  : " << bloodGroup
              << "\nDisease      : " << disease
              << "\nAddress      : " << address << '\n';
}

void Patient::printRow() const {
    std::cout << std::left << std::setw(12) << patientId
              << std::setw(24) << name.substr(0, 23)
              << std::setw(6) << age
              << std::setw(12) << gender.substr(0, 11)
              << std::setw(16) << phone.substr(0, 15)
              << std::setw(10) << bloodGroup.substr(0, 9)
              << std::setw(24) << disease.substr(0, 23) << '\n';
}
