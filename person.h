#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <utility>

// Base class for people in the hospital. Patient and Doctor reuse this common data.
class Person {
protected:
    int id;
    std::string name;
    int age;
    std::string gender;
    std::string phone;

public:
    Person() : id(0), age(0) {}

    Person(int id, std::string name, int age, std::string gender, std::string phone)
        : id(id),
          name(std::move(name)),
          age(age),
          gender(std::move(gender)),
          phone(std::move(phone)) {}

    virtual ~Person() = default;

    int getId() const {
        return id;
    }

    const std::string &getName() const {
        return name;
    }

    int getAge() const {
        return age;
    }

    const std::string &getGender() const {
        return gender;
    }

    const std::string &getPhone() const {
        return phone;
    }

    void setName(const std::string &value) {
        name = value;
    }

    void setAge(int value) {
        age = value;
    }

    void setGender(const std::string &value) {
        gender = value;
    }

    void setPhone(const std::string &value) {
        phone = value;
    }

    virtual std::string serialize() const = 0;
    virtual void showDetails() const = 0;
};

#endif
