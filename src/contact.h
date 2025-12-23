#ifndef CONTACT_H
#define CONTACT_H

#include <QString>


struct Contact {
    int id;
    QString firstName;
    QString lastName;
    QString email;
    QString phone;
    QString city;
    QString country;

    Contact() : id(-1) {}

    Contact(int id, const QString &firstName, const QString &lastName,
            const QString &email, const QString &phone,
            const QString &city = "", const QString &country = "")
        : id(id), firstName(firstName), lastName(lastName),
          email(email), phone(phone), city(city), country(country) {}

    bool isValid() const {
        return !firstName.isEmpty() && !lastName.isEmpty();
    }

    QString fullName() const {
        return firstName + " " + lastName;
    }
};

#endif // CONTACT_H
