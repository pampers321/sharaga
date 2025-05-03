#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <algorithm>  // std::swap

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
      : title(t), author(a), year(y),
        publisher(pub), pages(p), next(nullptr) {}
};

// Головной элемент списка
struct BookList {
    BookNode* head;
    size_t count;
    BookList() : head(nullptr), count(0) {}
};

// Вспомогательный ввод целого числа
int readInt(const std::string& prompt) {
    int x;
    while (true) {
        std::cout << prompt;
        if (std::cin >> x) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return x;
        }
        std::cout << "Неверный ввод, попробуйте ещё раз.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// ==== Базовые операции с узлами ====

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

// Добавить после элемента с данным заголовком
bool addAfter(BookList& L, const std::string& keyTitle, BookNode* node) {
    for (BookNode* cur = L.head; cur; cur = cur->next) {
        if (cur->title == keyTitle) {
            node->next = cur->next;
            cur->next = node;
            ++L.count;
            return true;
        }
    }
    return false;
}

// Удалить по заголовку (первое вхождение)
bool removeByTitle(BookList& L, const std::string& keyTitle) {
    BookNode* cur = L.head;
    BookNode* prev = nullptr;
    while (cur) {
        if (cur->title == keyTitle) {
            if (!prev) L.head = cur->next;
            else       prev->next = cur->next;
            delete cur;
            --L.count;
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false;
}

// Поиск по заголовку (возвращает указатель или nullptr)
BookNode* findByTitle(BookList& L, const std::string& key) {
    for (BookNode* cur = L.head; cur; cur = cur->next)
        if (cur->title == key)
            return cur;
    return nullptr;
}

// Вывод по автору
void findByAuthor(BookList& L, const std::string& key) {
    bool found = false;
    for (BookNode* cur = L.head; cur; cur = cur->next) {
        if (cur->author == key) {
            std::cout << "  «" << cur->title << "», "
                      << cur->year << ", " << cur->publisher
                      << ", " << cur->pages << " стр.\n";
            found = true;
        }
    }
    if (!found) std::cout << "Не найдено книг автора «" << key << "»\n";
}

// Вывод по году
void findByYear(BookList& L, int key) {
    bool found = false;
    for (BookNode* cur = L.head; cur; cur = cur->next) {
        if (cur->year == key) {
            std::cout << "  «" << cur->title << "», "
                      << cur->author << ", " << cur->publisher
                      << ", " << cur->pages << " стр.\n";
            found = true;
        }
    }
    if (!found) std::cout << "Не найдено книг за " << key << " год\n";
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

// ==== Сохранение/загрузка в бинарный файл ====

// Сравнение узлов (все поля)
bool equals(const BookNode* A, const BookNode* B) {
    return A->title     == B->title
        && A->author    == B->author
        && A->year      == B->year
        && A->publisher == B->publisher
        && A->pages     == B->pages;
}

// Сохранить весь список в файл (перезапись)
void saveToFile(const BookList& L, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Не удалось открыть файл для записи\n";
        return;
    }
    out.write(reinterpret_cast<const char*>(&L.count), sizeof(L.count));
    for (BookNode* cur = L.head; cur; cur = cur->next) {
        auto wrs = [&](const std::string& s){
            uint32_t len = uint32_t(s.size());
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(s.data(), len);
        };
        wrs(cur->title);
        wrs(cur->author);
        out.write(reinterpret_cast<const char*>(&cur->year), sizeof(cur->year));
        wrs(cur->publisher);
        out.write(reinterpret_cast<const char*>(&cur->pages), sizeof(cur->pages));
    }
    std::cout << "Сохранено в «" << filename << "» (" << L.count << " книг)\n";
}

// Загрузить из файла (полная перезагрузка списка)
void loadFromFile(BookList& L, const std::string& filename) {
    // очистка текущего
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
        auto rds = [&](void)->std::string {
            uint32_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string s(len, '\0');
            in.read(&s[0], len);
            return s;
        };
        std::string t = rds();
        std::string a = rds();
        int y; in.read(reinterpret_cast<char*>(&y), sizeof(y));
        std::string p = rds();
        int pg; in.read(reinterpret_cast<char*>(&pg), sizeof(pg));
        addBack(L, new BookNode(t,a,y,p,pg));
    }
    std::cout << "Загружено из «" << filename << "» (" << L.count << " книг)\n";
}

// Слияние: добавляем в список только тех книг из файла, которых ещё нет
void mergeFromFile(BookList& L, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Не удалось открыть файл для чтения\n";
        return;
    }
    size_t n; in.read(reinterpret_cast<char*>(&n), sizeof(n));
    size_t added = 0;
    for (size_t i = 0; i < n; ++i) {
        auto rds = [&](void)->std::string {
            uint32_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string s(len, '\0');
            in.read(&s[0], len);
            return s;
        };
        std::string t = rds();
        std::string a = rds();
        int y; in.read(reinterpret_cast<char*>(&y), sizeof(y));
        std::string p = rds();
        int pg; in.read(reinterpret_cast<char*>(&pg), sizeof(pg));
        BookNode temp(t,a,y,p,pg);

        bool exists = false;
        for (BookNode* cur = L.head; cur; cur = cur->next) {
            if (equals(cur, &temp)) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            addBack(L, new BookNode(t,a,y,p,pg));
            ++added;
        }
    }
    std::cout << "Добавлено новых книг: " << added << "\n";
}

// ==== Сортировка «пузырьком» по полям ====

void swapData(BookNode* A, BookNode* B) {
    std::swap(A->title,     B->title);
    std::swap(A->author,    B->author);
    std::swap(A->year,      B->year);
    std::swap(A->publisher, B->publisher);
    std::swap(A->pages,     B->pages);
}

// key = 't' (title), 'a' (author), 'y' (year)
void sortList(BookList& L, char key) {
    if (!L.head || !L.head->next) return;
    bool swapped;
    do {
        swapped = false;
        BookNode* cur = L.head;
        while (cur->next) {
            bool need = false;
            if (key=='t' && cur->title > cur->next->title)        need = true;
            if (key=='a' && cur->author > cur->next->author)      need = true;
            if (key=='y' && cur->year > cur->next->year)          need = true;
            if (need) {
                swapData(cur, cur->next);
                swapped = true;
            }
            cur = cur->next;
        }
    } while (swapped);
}

// ==== Меню и main ====

int main() {
    BookList library;
    const std::string filename = "books.bin";

    while (true) {
        std::cout << "\n=== Меню ===\n"
                  << "1) Добавить книгу в начало\n"
                  << "2) Добавить книгу в конец\n"
                  << "3) Добавить после заголовка\n"
                  << "4) Удалить по заголовку\n"
                  << "5) Поиск по заголовку\n"
                  << "6) Поиск по автору\n"
                  << "7) Поиск по году\n"
                  << "8) Показать все книги\n"
                  << "9) Сохранить в файл\n"
                  << "10) Загрузить из файла\n"
                  << "11) Добавить новые из файла\n"
                  << "12) Сортировать по названию\n"
                  << "13) Сортировать по автору\n"
                  << "14) Сортировать по году\n"
                  << "0) Выход\n"
                  << "Выберите пункт: ";
        int choice;
        if (!(std::cin >> choice)) break;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string t, a, pub, key;
        int y, pg;
        bool ok;

        switch (choice) {
          case 0:
            goto EXIT;
          case 1:
          case 2:
            std::cout << "Заголовок: ";       std::getline(std::cin, t);
            std::cout << "Автор: ";           std::getline(std::cin, a);
            y  = readInt("Год: ");
            std::cout << "Издательство: ";    std::getline(std::cin, pub);
            pg = readInt("Страниц: ");
            if (choice==1) addFront(library, new BookNode(t,a,y,pub,pg));
            else           addBack (library, new BookNode(t,a,y,pub,pg));
            break;
          case 3:
            std::cout << "После какого заголовка? "; std::getline(std::cin, key);
            std::cout << "Новая книга:\nЗаголовок: "; std::getline(std::cin, t);
            std::cout << "Автор: ";                   std::getline(std::cin, a);
            y  = readInt("Год: ");
            std::cout << "Издательство: ";            std::getline(std::cin, pub);
            pg = readInt("Страниц: ");
            ok = addAfter(library, key, new BookNode(t,a,y,pub,pg));
            if (!ok) std::cout << "Книга «" << key << "» не найдена.\n";
            break;
          case 4:
            std::cout << "Какой заголовок удалить? "; std::getline(std::cin, key);
            if (!removeByTitle(library, key))
                std::cout << "Книга не найдена.\n";
            break;
          case 5:
            std::cout << "Искомый заголовок: "; std::getline(std::cin, key);
            if (BookNode* n = findByTitle(library, key)) {
                std::cout << "Найдена: «" << n->title << "», "
                          << n->author << ", " << n->year << "\n";
            } else {
                std::cout << "Не найдена.\n";
            }
            break;
          case 6:
            std::cout << "Искомый автор: "; std::getline(std::cin, key);
            findByAuthor(library, key);
            break;
          case 7:
            y = readInt("Год: ");
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
          case 12:
            sortList(library, 't');
            std::cout << "Отсортировано по названию.\n";
            break;
          case 13:
            sortList(library, 'a');
            std::cout << "Отсортировано по автору.\n";
            break;
          case 14:
            sortList(library, 'y');
            std::cout << "Отсортировано по году издания.\n";
            break;
          default:
            std::cout << "Неверный пункт меню.\n";
        }
    }
EXIT:
    while (library.head) {
        BookNode* tmp = library.head;
        library.head = library.head->next;
        delete tmp;
    }
    return 0;
}
