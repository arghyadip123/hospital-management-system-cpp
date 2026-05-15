#ifndef STORAGE_UTILS_H
#define STORAGE_UTILS_H

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace Storage {

// Removes surrounding spaces so file records and console input stay consistent.
inline std::string trim(const std::string &value) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = value.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }

    const std::size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

inline std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

inline std::string toUpper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    return value;
}

inline bool isDigits(const std::string &value) {
    return !value.empty() &&
           std::all_of(value.begin(), value.end(),
                       [](unsigned char ch) { return std::isdigit(ch) != 0; });
}

inline std::string idYear() {
    return "2026";
}

inline std::string formatProfessionalId(const std::string &prefix, int sequenceNumber) {
    if (sequenceNumber <= 0) {
        return "";
    }

    std::ostringstream stream;
    stream << prefix << idYear() << std::setw(3) << std::setfill('0') << sequenceNumber;
    return stream.str();
}

inline bool parsePositiveInt(const std::string &text, int &output) {
    if (!isDigits(text)) {
        return false;
    }

    std::stringstream stream(text);
    return (stream >> output) && output > 0;
}

inline std::string normalizeProfessionalId(const std::string &value, const std::string &prefix) {
    const std::string normalized = toUpper(trim(value));
    const std::string normalizedPrefix = toUpper(prefix);
    int sequenceNumber = 0;

    if (normalized.rfind(normalizedPrefix, 0) == 0) {
        const std::string suffix = normalized.substr(normalizedPrefix.length());
        if (suffix.length() < 7 || suffix.substr(0, 4) != idYear()) {
            return "";
        }

        if (!parsePositiveInt(suffix.substr(4), sequenceNumber)) {
            return "";
        }
    } else if (isDigits(normalized) && normalized.rfind(idYear(), 0) == 0 && normalized.length() > 4) {
        if (!parsePositiveInt(normalized.substr(4), sequenceNumber)) {
            return "";
        }
    } else if (!parsePositiveInt(normalized, sequenceNumber)) {
        return "";
    }

    return formatProfessionalId(normalizedPrefix, sequenceNumber);
}

inline int professionalIdNumber(const std::string &value, const std::string &prefix) {
    const std::string normalized = normalizeProfessionalId(value, prefix);
    if (normalized.empty()) {
        return 0;
    }

    int number = 0;
    std::stringstream stream(normalized.substr(prefix.length() + idYear().length()));
    stream >> number;
    return number;
}

inline std::string formatPatientId(int sequenceNumber) {
    return formatProfessionalId("HSP", sequenceNumber);
}

inline std::string normalizePatientId(const std::string &value) {
    const std::string normalized = toUpper(trim(value));
    std::string numberText;
    int sequenceNumber = 0;

    if (normalized.rfind("HSP", 0) == 0) {
        const std::string hospitalPart = normalized.substr(3);
        if (hospitalPart.length() < 7) {
            return "";
        }

        const std::string yearText = hospitalPart.substr(0, 4);
        const std::string sequenceText = hospitalPart.substr(4);
        if (yearText != "2026" || !isDigits(sequenceText)) {
            return "";
        }

        std::stringstream stream(sequenceText);
        if (!(stream >> sequenceNumber) || sequenceNumber <= 0) {
            return "";
        }
    } else if (normalized.rfind("PAT", 0) == 0) {
        numberText = normalized.substr(3);
        if (!isDigits(numberText)) {
            return "";
        }

        std::stringstream stream(numberText);
        if (!(stream >> sequenceNumber) || sequenceNumber <= 0) {
            return "";
        }

        // Compatibility with the previous PAT1001 format.
        if (sequenceNumber >= 1000) {
            sequenceNumber -= 1000;
        }
    } else {
        numberText = normalized;
        if (!isDigits(numberText)) {
            return "";
        }

        if (numberText.rfind("2026", 0) == 0 && numberText.length() > 4) {
            const std::string sequenceText = numberText.substr(4);
            if (!isDigits(sequenceText)) {
                return "";
            }

            std::stringstream stream(sequenceText);
            if (!(stream >> sequenceNumber) || sequenceNumber <= 0) {
                return "";
            }
        } else {
            std::stringstream stream(numberText);
            if (!(stream >> sequenceNumber) || sequenceNumber <= 0) {
                return "";
            }

            // Compatibility with old numeric records and old search habits.
            if (sequenceNumber >= 1000 && sequenceNumber < 2000) {
                sequenceNumber -= 1000;
            }
        }
    }

    if (sequenceNumber <= 0) {
        return "";
    }

    return formatPatientId(sequenceNumber);
}

inline int patientIdNumber(const std::string &value) {
    const std::string normalized = normalizePatientId(value);
    if (normalized.empty()) {
        return 0;
    }

    int number = 0;
    std::stringstream stream(normalized.substr(7));
    stream >> number;
    return number;
}

inline std::string formatDoctorId(int sequenceNumber) {
    return formatProfessionalId("DOC", sequenceNumber);
}

inline std::string normalizeDoctorId(const std::string &value) {
    return normalizeProfessionalId(value, "DOC");
}

inline int doctorIdNumber(const std::string &value) {
    return professionalIdNumber(value, "DOC");
}

inline std::string formatAppointmentId(int sequenceNumber) {
    return formatProfessionalId("APT", sequenceNumber);
}

inline std::string normalizeAppointmentId(const std::string &value) {
    return normalizeProfessionalId(value, "APT");
}

inline int appointmentIdNumber(const std::string &value) {
    return professionalIdNumber(value, "APT");
}

inline std::string formatBillId(int sequenceNumber) {
    return formatProfessionalId("BIL", sequenceNumber);
}

inline std::string normalizeBillId(const std::string &value) {
    return normalizeProfessionalId(value, "BIL");
}

inline int billIdNumber(const std::string &value) {
    return professionalIdNumber(value, "BIL");
}

inline std::tm localTime() {
    const std::time_t now = std::time(nullptr);
    std::tm local = {};

#ifdef _WIN32
    local = *std::localtime(&now);
#else
    localtime_r(&now, &local);
#endif

    return local;
}

inline std::string currentDate() {
    const std::tm local = localTime();
    std::ostringstream stream;
    stream << std::put_time(&local, "%Y-%m-%d");
    return stream.str();
}

inline std::string currentTime() {
    const std::tm local = localTime();
    std::ostringstream stream;
    stream << std::put_time(&local, "%H:%M");
    return stream.str();
}

inline std::string currentDateTime() {
    const std::tm local = localTime();
    std::ostringstream stream;
    stream << std::put_time(&local, "%Y-%m-%d %H:%M");
    return stream.str();
}

inline std::string safeFileName(std::string value) {
    value = trim(value);
    for (char &ch : value) {
        const bool safe = std::isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_';
        if (!safe) {
            ch = '_';
        }
    }
    return value.empty() ? "record" : value;
}

// Replaces unsafe record separators before text is written to a pipe-delimited file.
inline std::string cleanField(std::string value) {
    for (char &ch : value) {
        if (ch == '|') {
            ch = '/';
        } else if (ch == '\n' || ch == '\r') {
            ch = ' ';
        }
    }

    return trim(value);
}

inline std::vector<std::string> splitRecord(const std::string &line) {
    std::vector<std::string> fields;
    std::string field;

    for (char ch : line) {
        if (ch == '|') {
            fields.push_back(field);
            field.clear();
        } else {
            field += ch;
        }
    }

    fields.push_back(field);
    return fields;
}

inline std::string joinFields(const std::vector<std::string> &fields) {
    std::string record;

    for (std::size_t index = 0; index < fields.size(); ++index) {
        if (index > 0) {
            record += "|";
        }
        record += cleanField(fields[index]);
    }

    return record;
}

inline bool parseInt(const std::string &text, int &output) {
    std::stringstream stream(trim(text));
    int value = 0;
    char extra = '\0';

    if (stream >> value && !(stream >> extra)) {
        output = value;
        return true;
    }

    return false;
}

inline bool parseDouble(const std::string &text, double &output) {
    std::stringstream stream(trim(text));
    double value = 0.0;
    char extra = '\0';

    if (stream >> value && !(stream >> extra)) {
        output = value;
        return true;
    }

    return false;
}

inline std::string formatMoney(double amount) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << amount;
    return stream.str();
}

} // namespace Storage

#endif
