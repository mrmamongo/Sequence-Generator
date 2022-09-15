//
// Created by MrMam on 12.09.2022.
//

#ifndef SEQ_GENERATOR_SEQ_STORAGE_HXX
#define SEQ_GENERATOR_SEQ_STORAGE_HXX

#include "sequence.hxx"
#include <map>
#include <mutex>
#include <utils/constants.hxx>
#include <vector>

class seq_storage {
public:
/// \brief Реализация паттерна Singleton с помощью Double-Checked Locking
/// \return Указатель на единственный экземпляр класса
    static seq_storage *instance() {
        seq_storage *tmp = storage;
        std::atomic_thread_fence(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(_mutex);
            tmp = storage;
            if (tmp == nullptr) {
                tmp = new seq_storage;
                std::atomic_thread_fence(std::memory_order_release);
                storage = tmp;
            }
        }
        return tmp;
    }

    seq_storage() = default;

    ~seq_storage() = default;

    seq_storage(const seq_storage &) = delete;

    seq_storage(seq_storage &&) = delete;

    seq_storage &operator=(const seq_storage &) = delete;

    seq_storage &operator=(seq_storage &&) = delete;

public:

/// Парсит запрос и добавляет последовательность в хранилище
/// \param name IP-Адрес клиента
/// \param query Строка - запрос
/// \return
    bool add_sequence(const ip_address &addr, const std::string &query);

/// Создает пустую последовательность для клиента
/// \param name IP-Адрес клиента
/// \return
    bool create_sequence(const ip_address &addr);


/// \brief Стирает последовательность с заданным именем
/// \param name IP-Адрес клиента
/// \return
    bool remove_sequence(const ip_address &name);


/// \brief Проверяет, существует ли последовательность для данного адреса
/// \param name IP-Адрес клиента
/// \return
    bool contains(const ip_address &name) const;


/// \brief Возвращает следующие три значения последовательности
/// \param name IP-Адрес клиента
/// \return строка с тремя значениями последовательности
    std::vector<uint8_t> next(const ip_address &name);

private:
    static seq_storage *volatile storage;
    std::map<ip_address, std::array<sequence, 3>> _storage;
    static std::mutex _mutex;
};


#endif //SEQ_GENERATOR_SEQ_STORAGE_HXX
