#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <memory>
#include <limits> // Для std::numeric_limits

using namespace std;

// Класс для проверки пароля
class PasswordValidator {
public:
    virtual bool validate(const string& password) const = 0;
    virtual ~PasswordValidator() = default;
};

// Конкретный валидатор для сложного пароля
class ComplexPasswordValidator : public PasswordValidator {
public:
    bool validate(const string& password) const override {
        if (password.length() < 8) {
            return false;
        }

        bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
        string specialChars = "?@*_+-%&";

        for (char c : password) {
            if (isdigit(c)) hasDigit = true;
            else if (islower(c)) hasLower = true;
            else if (isupper(c)) hasUpper = true;
            else if (specialChars.find(c) != string::npos) hasSpecial = true;
        }

        return hasLower && hasUpper && hasDigit && hasSpecial;
    }
};

// Класс Singleton для управления паролем в файле
class PasswordManager {
private:
    string fileName;
    static PasswordManager* instance;

    PasswordManager(const string& file) : fileName(file) {}

public:
    static PasswordManager* getInstance(const string& file = "password.txt") {
        if (!instance) {
            instance = new PasswordManager(file);
        }
        return instance;
    }

    void savePassword(const string& password) {
        ofstream outFile(fileName);
        if (outFile.is_open()) {
            outFile << password;
            outFile.close();
        } else {
            throw runtime_error("Не удалось открыть файл для записи.");
        }
    }

    string loadPassword() {
        ifstream inFile(fileName);
        string password;
        if (inFile.is_open()) {
            getline(inFile, password);
            inFile.close();
        } else {
            throw runtime_error("Не удалось открыть файл для чтения.");
        }
        return password;
    }

    bool isPasswordFileEmpty() {
        ifstream inFile(fileName);
        return inFile.peek() == ifstream::traits_type::eof();
    }
};

PasswordManager* PasswordManager::instance = nullptr;

// Интерфейс команды
class Command {
public:
    virtual void execute() = 0;
    virtual ~Command() = default;
};

// Команда для установки нового пароля
class SetPasswordCommand : public Command {
    PasswordManager* manager;
    const PasswordValidator& validator;

public:
    SetPasswordCommand(PasswordManager* mgr, const PasswordValidator& val)
        : manager(mgr), validator(val) {}

    void execute() override {
        string password;
        cout << "Введите новый пароль: ";
        getline(cin, password);

        if (validator.validate(password)) {
            manager->savePassword(password);
            cout << "Пароль успешно сохранен!" << endl;
        } else {
            cout << "Пароль не соответствует требованиям." << endl;
        }
    }
};

// Команда для входа в систему
class LoginCommand : public Command {
    PasswordManager* manager;

public:
    LoginCommand(PasswordManager* mgr) : manager(mgr) {}

    void execute() override {
        string enteredPassword;
        cout << "Введите пароль: ";
        getline(cin, enteredPassword);

        string savedPassword = manager->loadPassword();
        if (enteredPassword == savedPassword) {
            cout << "Вход выполнен успешно!" << endl;
        } else {
            cout << "Неверный пароль!" << endl;
        }
    }
};
int main() {
    PasswordManager* manager = PasswordManager::getInstance();
    ComplexPasswordValidator validator;

    // Если файл с паролем пуст, требуется установка нового пароля
    if (manager->isPasswordFileEmpty()) {
        SetPasswordCommand setPassword(manager, validator);
        setPassword.execute();
    }

    while (true) {
        cout << "\nМеню:\n1. Войти в систему\n2. Установить новый пароль\n3. Выйти\nВыберите опцию: ";
        int choice;
        
        if (!(cin >> choice)) { // Проверка корректности ввода
            cin.clear(); // Сброс состояния потока
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Очистка буфера ввода
            cout << "Некорректный ввод. Попробуйте снова.\n";
            continue;
        }
        
        cin.ignore(); // Убираем символ новой строки после ввода числа

        try {
            switch (choice) {
                case 1: {
                    string enteredPassword;
                    cout << "Введите пароль: ";
                    getline(cin, enteredPassword); // Ввод пароля

                    string savedPassword = manager->loadPassword(); // Чтение сохранённого пароля
                    if (enteredPassword == savedPassword) {
                        cout << "Вход выполнен успешно!" << endl;
                    } else {
                        cout << "Неверный пароль!" << endl;
                    }
                    break;
                }
                case 2: {
                    SetPasswordCommand setPassword(manager, validator);
                    setPassword.execute();
                    break;
                }
                case 3:
                    cout << "Выход из программы." << endl;
                    return 0;
                default:
                    cout << "Некорректный выбор. Попробуйте снова." << endl;
            }
        } catch (const exception& ex) {
            cout << "Ошибка: " << ex.what() << endl;
        }
    }

    return 0;
}
