#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>

class Appointment {
private:
    std::string appointmentId;
    std::string patientId;
    std::string doctorId;
    std::string date;
    std::string time;
    std::string reason;
    std::string status;

public:
    Appointment();
    Appointment(std::string appointmentId, std::string patientId, std::string doctorId, std::string date,
                std::string time, std::string reason, std::string status);

    const std::string &getAppointmentId() const;
    const std::string &getPatientId() const;
    const std::string &getDoctorId() const;
    const std::string &getDate() const;
    const std::string &getTime() const;
    const std::string &getReason() const;
    const std::string &getStatus() const;

    bool isScheduled() const;
    void setStatus(const std::string &value);
    void cancel();

    std::string serialize() const;
    static Appointment fromRecord(const std::string &line, bool &ok);
};

#endif
