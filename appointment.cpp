#include "appointment.h"

#include "storage_utils.h"

#include <utility>
#include <vector>

Appointment::Appointment()
    : status("Scheduled") {}

Appointment::Appointment(std::string appointmentId, std::string patientId, std::string doctorId, std::string date,
                         std::string time, std::string reason, std::string status)
    : appointmentId(std::move(appointmentId)),
      patientId(std::move(patientId)),
      doctorId(std::move(doctorId)),
      date(std::move(date)),
      time(std::move(time)),
      reason(std::move(reason)),
      status(std::move(status)) {}

const std::string &Appointment::getAppointmentId() const {
    return appointmentId;
}

const std::string &Appointment::getPatientId() const {
    return patientId;
}

const std::string &Appointment::getDoctorId() const {
    return doctorId;
}

const std::string &Appointment::getDate() const {
    return date;
}

const std::string &Appointment::getTime() const {
    return time;
}

const std::string &Appointment::getReason() const {
    return reason;
}

const std::string &Appointment::getStatus() const {
    return status;
}

bool Appointment::isScheduled() const {
    return Storage::toLower(status) == "scheduled";
}

void Appointment::setStatus(const std::string &value) {
    status = value;
}

void Appointment::cancel() {
    status = "Cancelled";
}

std::string Appointment::serialize() const {
    return Storage::joinFields({appointmentId, patientId, doctorId, date, time, reason, status});
}

Appointment Appointment::fromRecord(const std::string &line, bool &ok) {
    const std::vector<std::string> fields = Storage::splitRecord(line);
    const std::string parsedAppointmentId = fields.empty() ? "" : Storage::normalizeAppointmentId(fields[0]);
    const std::string parsedPatientId = fields.size() > 1 ? Storage::normalizePatientId(fields[1]) : "";
    const std::string parsedDoctorId = fields.size() > 2 ? Storage::normalizeDoctorId(fields[2]) : "";

    ok = fields.size() >= 7 &&
         !parsedAppointmentId.empty() &&
         !parsedPatientId.empty() &&
         !parsedDoctorId.empty();

    if (!ok) {
        return Appointment();
    }

    return Appointment(parsedAppointmentId, parsedPatientId, parsedDoctorId,
                       fields[3], fields[4], fields[5], fields[6]);
}
