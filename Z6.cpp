#include <iostream>
#include <string>
#include <fstream>
#include <limits>

// Узловой элемент списка — книга
struct BookNode {
    std::string title;
    std::string author;
    int year;
    std::string publisher;
    int pages;
    BookNode* next;

    BookNode(const std::string& t, const std::string& a, int y,
             const std::string& pub, int p)
        : title(t), author(a), year(y), publisher(pub), pages(p), next(nullptr) {}
};

// Головной элемент списка
struct BookList {
    BookNode* head;
    size_t count;

    BookList() : head(nullptr), count(0) {}
};

// Прототипы
void addFront(BookList& L, BookNode* node);
void addBack(BookList& L, BookNode* node);
bool addAfter(BookList& L, const std::string& keyTitle, BookNode* node);
bool removeByTitle(BookList& L, const std::string& keyTitle);
BookNode* findByTitle(BookList& L, const std::string& key);
void findByAuthor(BookList& L, const std::string& key);
void findByYear(BookList& L, int key);
void saveToFile(BookList& L, const std::string& filename);
void loadFromFile(BookList& L, const std::string& filename);
void mergeFromFile(BookList& L, const std::string& filename);
bool equals(const BookNode* A, const BookNode* B);
void printList(const BookList& L);

// Вспомогательный ввод целого числа
int readInt(const std::string& prompt) {
    int x;
    while (true) {
        std::cout << prompt;
        if (std::cin >> x) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return x;
        } else {
            std::cout << "Неверный ввод, попробуйте ещё раз.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

// Добавить в начало
void addFront(BookList& L, BookNode* node) {
    node->next = L.head;
    L.head = node;
    ++L.count;
}

// Добавить в конец
void addBack(BookList& L, BookNode* node) {
    if (!L.head) {
        L.head = node;
    } else {
        BookNode* cur = L.head;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }
    ++L.count;
}

// Добавить после элемента с заданным заголовком
bool addAfter(BookList& L, const std::string& keyTitle, BookNode* node) {
    BookNode* cur = L.head;
    while (cur) {
        if (cur->title == keyTitle) {
            node->next = cur->next;
            cur->next = node;
            ++L.count;
            return true;
        }
        cur = cur->next;
    }
    return false;
}

// Удалить первый узел с данным заголовком
bool removeByTitle(BookList& L, const std::string& keyTitle) {
    BookNode* cur = L.head;
    BookNode* prev = nullptr;
    while (cur) {
        if (cur->title == keyTitle) {
            if (!prev) {
                L.head = cur->next;
            } else {
                prev->next = cur->next;
            }
            delete cur;
            --L.count;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

// Найти первое вхождение по заголовку
BookNode* findByTitle(BookList& L, const std::string& key) {
    BookNode* cur = L.head;
    while (cur) {
        if (cur->title == key)
            return cur;
        cur = cur->next;
    }
    return nullptr;
}

// Показать все книги данного автора
void findByAuthor(BookList& L, const std::string& key) {
    BookNode* cur = L.head;
    bool found = false;
    while (cur) {
        if (cur->author == key) {
            std::cout << "  " << cur->title << " (" << cur->year << "), "
                      << cur->publisher << ", " << cur->pages << " стр.\n";
            found = true;
        }
        cur = cur->next;
    }
    if (!found) std::cout << "Не найдено книг автора «" << key << "»\n";
}

// Показать все книги данного года
void findByYear(BookList& L, int key) {
    BookNode* cur = L.head;
    bool found = false;
    while (cur) {
        if (cur->year == key) {
            std::cout << "  " << cur->title << " — " << cur->author
                      << ", " << cur->publisher << ", " << cur->pages << " стр.\n";
            found = true;
        }
        cur = cur->next;
    }
    if (!found) std::cout << "Не найдено книг за " << key << " год\n";
}

// Сравнение двух книг по всем полям
bool equals(const BookNode* A, const BookNode* B) {
    return A->title == B->title
        && A->author == B->author
        && A->year == B->year
        && A->publisher == B->publisher
        && A->pages == B->pages;
}

// Сохранить список в бинарный файл
void saveToFile(BookList& L, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Не удалось открыть файл для записи\n";
        return;
    }
    // записать количество элементов
    out.write(reinterpret_cast<const char*>(&L.count), sizeof(L.count));
    // для каждой книги: длина строки + данные, etc.
    BookNode* cur = L.head;
    while (cur) {
        auto writeString = [&](const std::string& s) {
            uint32_t len = uint32_t(s.size());
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(s.data(), len);
        };
        writeString(cur->title);
        writeString(cur->author);
        out.write(reinterpret_cast<const char*>(&cur->year), sizeof(cur->year));
        writeString(cur->publisher);
        out.write(reinterpret_cast<const char*>(&cur->pages), sizeof(cur->pages));
        cur = cur->next;
    }
    std::cout << "Список сохранён в «" << filename << "» (" << L.count << " книг)\n";
}

// Полная загрузка из файла (перезатирает текущий список!)
void loadFromFile(BookList& L, const std::string& filename) {
    // сначала очистим список
    while (L.head) {
        BookNode* tmp = L.head;
        L.head = L.head->next;
        delete tmp;
    }
    L.count = 0;

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Не удалось открыть файл для чтения\n";
        return;
    }
    size_t n;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    for (size_t i = 0; i < n; ++i) {
        auto readString = [&]() {
            uint32_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string s(len, '\0');
            in.read(&s[0], len);
            return s;
        };
        std::string t = readString();
        std::string a = readString();
        int y; in.read(reinterpret_cast<char*>(&y), sizeof(y));
        std::string p = readString();
        int pg; in.read(reinterpret_cast<char*>(&pg), sizeof(pg));
        BookNode* node = new BookNode(t, a, y, p, pg);
        addBack(L, node);
    }
    std::cout << "Список загружен из «" << filename << "» (" << L.count << " книг)\n";
}

// Добавить из файла только новые (не совпадающие полностью)
void mergeFromFile(BookList& L, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Не удалось открыть файл для чтения\n";
        return;
    }
    size_t n;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    size_t added = 0;
    for (size_t i = 0; i < n; ++i) {
        auto readString = [&]() {
            uint32_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string s(len, '\0');
            in.read(&s[0], len);
            return s;
        };
        std::string t = readString();
        std::string a = readString();
        int y; in.read(reinterpret_cast<char*>(&y), sizeof(y));
        std::string p = readString();
        int pg; in.read(reinterpret_cast<char*>(&pg), sizeof(pg));
        BookNode temp(t,a,y,p,pg);
        // проверим, есть ли уже такая книга
        bool exists = false;
        for (BookNode* cur = L.head; cur; cur = cur->next) {
            if (equals(cur, &temp)) { exists = true; break; }
        }
        if (!exists) {
            addBack(L, new BookNode(t,a,y,p,pg));
            ++added;
        }
    }
    std::cout << "Добавлено новых книг из «" << filename << "»: " << added << "\n";
}

// Вывести весь список
void printList(const BookList& L) {
    std::cout << "Всего книг: " << L.count << "\n";
    size_t idx = 1;
    for (BookNode* cur = L.head; cur; cur = cur->next, ++idx) {
        std::cout << idx << ") «" << cur->title << "», "
                  << cur->author << ", " << cur->year << ", "
                  << cur->publisher << ", " << cur->pages << " стр.\n";
    }
}

int main() {
    BookList library;
    std::string filename = "books.bin";

    while (true) {
        std::cout << "\n=== Меню ===\n"
                  << "1) Добавить книгу в начало\n"
                  << "2) Добавить книгу в конец\n"
                  << "3) Добавить книгу после заданного заголовка\n"
                  << "4) Удалить книгу по заголовку\n"
                  << "5) Поиск по заголовку\n"
                  << "6) Поиск по автору\n"
                  << "7) Поиск по году издания\n"
                  << "8) Показать все книги\n"
                  << "9) Сохранить в файл\n"
                  << "10) Загрузить из файла (перезапись)\n"
                  << "11) Добавить из файла только новые\n"
                  << "0) Выход\n"
                  << "Выберите пункт: ";
        int choice;
        if (!(std::cin >> choice)) break;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) break;

        std::string t, a, pub, key;
        int y, pg;
        bool ok;
        BookNode* node;

        switch (choice) {
            case 1:
                std::cout << "Введите данные книги (заголовок, автор, год, издательство, страниц):\n";
                std::getline(std::cin, t);
                std::getline(std::cin, a);
                y = readInt("Год: ");
                std::getline(std::cin, pub);
                pg = readInt("Страниц: ");
                addFront(library, new BookNode(t,a,y,pub,pg));
                break;
            case 2:
                std::cout << "Введите данные книги (заголовок, автор, год, издательство, страниц):\n";
                std::getline(std::cin, t);
                std::getline(std::cin, a);
                y = readInt("Год: ");
                std::getline(std::cin, pub);
                pg = readInt("Страниц: ");
                addBack(library, new BookNode(t,a,y,pub,pg));
                break;
            case 3:
                std::cout << "После какого заголовка вставить? ";
                std::getline(std::cin, key);
                std::cout << "Введите данные новой книги:\n";
                std::getline(std::cin, t);
                std::getline(std::cin, a);
                y = readInt("Год: ");
                std::getline(std::cin, pub);
                pg = readInt("Страниц: ");
                ok = addAfter(library, key, new BookNode(t,a,y,pub,pg));
                if (!ok) std::cout << "Книга «" << key << "» не найдена.\n";
                break;
            case 4:
                std::cout << "Какой заголовок удалить? ";
                std::getline(std::cin, key);
                ok = removeByTitle(library, key);
                if (!ok) std::cout << "Книга не найдена.\n";
                break;
            case 5:
                std::cout << "Введите заголовок для поиска: ";
                std::getline(std::cin, key);
                node = findByTitle(library, key);
                if (node) {
                    std::cout << "Найдена: «" << node->title << "», "
                              << node->author << ", " << node->year << ", "
                              << node->publisher << ", " << node->pages << " стр.\n";
                } else {
                    std::cout << "Не найдена.\n";
                }
                break;
            case 6:
                std::cout << "Введите автора для поиска: ";
                std::getline(std::cin, key);
                findByAuthor(library, key);
                break;
            case 7:
                y = readInt("Введите год: ");
                findByYear(library, y);
                break;
            case 8:
                printList(library);
                break;
            case 9:
                saveToFile(library, filename);
                break;
            case 10:
                loadFromFile(library, filename);
                break;
            case 11:
                mergeFromFile(library, filename);
                break;
            default:
                std::cout << "Неверный пункт\n";
        }
    }

    // очистка памяти
    while (library.head) {
        BookNode* tmp = library.head;
        library.head = library.head->next;
        delete tmp;
    }

    return 0;
}
