#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>


// Пара значений дискретной случайной 
struct Pair {
    Pair(double X = 0, double P = 0)
    {
        x = X;
        p = P;
    }
    Pair(const Pair& a)
    {
        x = a.x;
        p = a.p;
    }
    Pair& operator= (const Pair& Data)
    {
        x = Data.x;
        p = Data.p;
        return *this;
    }
    friend std::ostream& operator<< (std::ostream& stream, const Pair& item)
    {
        stream << "(" << item.x << "," << item.p << ")";
        return stream;
    }
    friend std::istream& operator>> (std::istream& stream, Pair& item)
    {
        stream >> item.x >> item.p;
        return stream;
    }

    // x - Значение аргумента
    double x;
    // p - Значение функции
    double p;
};


// Представление дискретной случайной величины
class Rand {
public:
    /* Конструктор из массива и длины
    * @param array - Исходный массив
    * @param length - Длина исходного массива
    * @param reserve - Число резервируемых ячеек
    */
    Rand(Pair* array, int length, int reserve = 10)
    {
        this->length = length;
        this->reserve = reserve;
        this->capacity = length + reserve;
        this->array = this->recreateArray(array, length, this->capacity);
    }

    /* Конструктор копирования
    * @param rand - Объект Rand
    */
    Rand(const Rand& rand) {
        this->length = rand.length;
        this->reserve = rand.reserve;
        this->capacity = rand.capacity;
        this->array = this->recreateArray(rand.array, this->length, this->capacity);
    }

    /* Конструктор по указателям (итераторам)
    * @param firstPointer - Указатель на начало исходного массива
    * @param secondPointer - Указатель на конец исходного массива
    * @param reserve - Число резервируемых ячеек
    */
    Rand(Pair* firstPointer, Pair* secondPointer, int reserve = 10)
    {
        this->length = secondPointer - firstPointer;
        this->reserve = reserve;
        this->capacity = this->length + this->reserve;
        this->array = this->recreateArray(firstPointer, this->length, this->capacity);
    }

    ~Rand()
    {
        delete[] this->array;
    }

    /*
    * @return - Фактическая длина массива
    */
    int getCapacity() { return capacity; }

    /*
    * @return - Число элементов в массива
    */
    int getLength() { return length; }

    /*
    * Добавляет элемент в конец массива.
    * @param value - Новый элемент массива
    */
    void push(Pair value)
    {
        // Добавляем пустые ячейки.
        if (this->length == this->capacity) {
            this->capacity += this->reserve;
            this->array = this->recreateArray(this->array, this->length, this->capacity, true);
        }
        this->array[length] = value;
        this->length += 1;
    }

    /*
    * Удаляет последний элемент массива.
    * @return Удаленный элемент
    */
    Pair pop() throw(std::out_of_range)
    {
        if (this->length == 0)
            throw std::out_of_range("Массив уже пуст");
        Pair deletedValue = this->array[this->length];
        this->length -= 1;
        // Удаляем пустые ячейки, если их слишком много.
        if (this->capacity - this->length > this->reserve) {
            this->capacity = this->length + this->reserve;
            this->array = this->recreateArray(this->array, this->length, this->capacity, true);
        }
        return deletedValue;
    }

    Pair& operator[] (const int index) throw(std::out_of_range)
    {
        if (index < 0 || index >= this->length)
            throw std::out_of_range("Индекс за пределами массива");
        return this->array[index];
    }

    Rand& operator= (const Rand& item)
    {
        if (this != &item) {
            this->capacity = item.capacity;
            this->length = item.length;
            this->array = this->recreateArray(item.array, item.length, item.capacity);
        }
        return *this;
    }

    friend std::ostream& operator<< (std::ostream& os, Rand& rand) {
        std::cout << "argument ";
        for (int i = 0; i < rand.length; i++) {
            os << rand.array[i].x << " ";
        }
        os << std::endl << "function ";
        for (int i = 0; i < rand.length; i++) {
            os << rand.array[i].p << " ";
        }
        os << " MO X= " << rand.getMean('x') << " ";
        os << "Disp X= " << rand.getDispersion('x') << " ";
        os << "MO Y= " << rand.getMean('y') << " ";
        os << "Disp Y= " << rand.getDispersion('y') << " ";
        os << "MonteKarlo= " << rand.getMonteCarlo();
        return os;
    }

    /*
    * Дружественный класс итератор
    */
    class Iterator {
    public:
        friend class Rand;
        Iterator(Pair* element) : current(element) {}
        Iterator(const Iterator& it) : current(it.current) {}
        bool operator== (const Iterator& it) { return this->current == it.current; }
        bool operator!= (const Iterator& it) { return this->current != it.current; }
        Iterator& operator++() { if (this->current != 0) this->current = this->current + 1; return *this; }
        Iterator& operator--() { if (this->current != 0) this->current = this->current + 1; return *this; }
        Iterator& operator+(int n)
        {
            Iterator result = this->current;
            for (int i = 0; i < n; i++)
                result.current = result.current + 1;
            return result;
        }
        Iterator& operator-(int n)
        {
            Iterator result = this->current;
            for (int i = 0; i < n; i++)
                result.current = result.current - 1;
            return result;
        }
        Pair& operator*() const
        {
            if (this->current != 0) return *this->current;
            else throw;
        }
    private:
        Pair* current;
    };

    /*
    * @return - Итератор на первый элемент массива
    */
    Iterator begin() { return this->array; }
    /*
    * @return - Итератор на конец массива
    */
    Iterator end() { return this->array + this->length; }

    /*
    * @param varType - Тип значения мат. ожидания ('x', 'y' или 'p')
    * @param inSquare - Флаг получения мат ожидания от квадрата величины
    * @return - Математическое ожидание
    */
    double getMean(char varType = 'x', bool inSquare = false)
    {
        double mean = 0;
        for (int i = 0; i < this->length; i++) {
            double value;
            if (varType == 'x')
                value = this->array[i].x;
            else if (varType == 'y' || varType == 'p')
                value = this->array[i].p;
            else
                throw std::invalid_argument("Неверное значение параметра varType");
            
            if (inSquare)
                value *= value;
            mean += (1.0 / this->length) * value;
        }
        return mean;
    }

    /*
    * @param varType - Тип значения мат. ожидания ('x', 'y' или 'p')
    * @return - Дисперсия
    */
    double getDispersion(char varType = 'x')
    {
        if (!(varType == 'x' || varType == 'y' || varType == 'p'))
            throw std::invalid_argument("Неверное значение параметра varType");

        double dispersion = this->getMean(varType, true) - pow(this->getMean(varType), 2);
        return dispersion;
    }

    /*
    * @return - значения интеграла методом Монте-Карло
    */
    double getMonteCarlo()
    {
        double sumP = 0;
        for (int i = 0; i < this->length; i++)
            sumP += this->array[i].p;

        const double a = this->array[0].x;
        const double b = this->array[this->length - 1].x;
        const double coef = (b - a) / this->length;

        return coef * sumP;
    }

private:
    /*
    * Пересоздает массив
    * @return - Новый динамический массив
    */
    Pair* recreateArray(Pair* array, int length, int capacity, bool clear = false) throw(std::bad_alloc)
    {
        try {
            Pair* newArray = new Pair[capacity];
            for (int i = 0; i < length; i++)
                newArray[i] = array[i];
            if (clear)
                delete[] array;
            return newArray;
        }
        catch (std::bad_alloc e) {
            std::cout << "Ошибка выделения памяти " << e.what() << std::endl;
            exit(0);
        }
        
    }

    // Число элементов в массиве.
    int length;

    // Фактическая длина массива.
    int capacity;

    // Число резервируемых ячеек при обновлении массива.
    int reserve;

    // Массив.
    Pair* array;
};


int main()
{
    setlocale(LC_ALL, "rus");
    int n;
    std::cin >> n;
    Pair* inputArray = new Pair[n];
    for (int i = 0; i < n; i++)
        std::cin >> inputArray[i].x;
    for (int i = 0; i < n; i++)
        std::cin >> inputArray[i].p;
    Rand rand = Rand(inputArray, n);
    std::cout << "input= " << rand;

    return 0;
}