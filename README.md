# 🧠 memory_allocator

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![AVL](https://img.shields.io/badge/структуры-AVL--tree-orange)](#-как-это-работает)
[![LD_PRELOAD](https://img.shields.io/badge/Linux-LD__PRELOAD-yellow)](#-сборка-и-запуск)

Экспериментальный менеджер памяти на C++, перехватывающий `malloc` / `free` / `realloc` / `calloc` через `LD_PRELOAD` и обслуживающий запросы за **O(log n)** на AVL-деревьях.

Ключевой инвариант: внутри аллокатора **ни одного вызова libc-аллокации** — собственные `set`/`map` на AVL-деревьях хранят узлы в статических массивах с пулом свободных слотов. Иначе `malloc` рекурсивно вызывал бы сам себя.

## 🧩 Как это работает

```
запрос malloc(n)
      │
      ▼
arena 512 МБ (однократный sbrk)
      │
      ▼
c_set<mem_block_t> ──lower_bound──► best-fit блок ≥ n   (O(log n), AVL по (size, address))
      │                                                адрес уходит в c_map<void*, size_t>
      ▼
изъятый блок разбивается: остаток возвращается в c_set
```

- **Поиск блока:** best-fit — минимальный свободный блок, чей размер ≥ запрошенного. При равных размерах выбирается блок с меньшим адресом (компаратор `(size, address)`), что держит аллокации ближе к началу арены
- **Свободные блоки:** `c_set<mem_block_t>` — AVL-дерево
- **Выделенные блоки:** `c_map<void*, size_t>` — AVL-дерево «указатель → размер»
- **Потокобезопасность:** глобальный `pthread_mutex_t` вокруг каждого вызова
- **Логирование:** прямые `open`/`write` в `malloc_log.txt` — `stdio`/iostream нельзя, они сами вызывают `malloc`
- **Узлы деревьев:** массивы фиксированного размера + free-list, без динамической памяти

## 📁 Структура проекта

```
memory_allocator/
├── malloc.cc     # точки входа malloc/free/realloc/calloc, c_mem (арена), логгер
├── structs.hh    # c_set и c_map на AVL-деревьях + Optional<T>
├── logger.hh     # ранняя iostream-версия логгера (в аллокаторе не используется)
└── README.md
```

## 🔨 Сборка и запуск

```bash
# сборка разделяемой библиотеки
g++ -o malloc.so -fPIC -shared -pthread malloc.cc

# запуск любой программы с подменённым аллокатором
LD_PRELOAD=./malloc.so ls

# лог аллокаций появится в ./malloc_log.txt
cat malloc_log.txt
```

Пример записи лога:

```
[malloc] size=72704, allocated at 0x560f580e0000, total_mem=536870912, free_mem=536798208, free%=0.99986
```

## ✅ Что проверено

- `LD_PRELOAD=./malloc.so ls` и другие системные утилиты — работают, лог пишется
- `malloc` / `free` / `realloc` / `calloc` — базовые сценарии через тестовый бинарь

## ⚠️ Ограничения (честный список)

- Нет слияния (coalescing) смежных свободных блоков при `free` → со временем растёт внешняя фрагментация
- Фиксированная арена 512 МБ и лимит 1024 узлов в каждом дереве — превышение бросает исключение
- `realloc` всегда аллоцирует новый блок и копирует данные (в т.ч. когда блок можно расширить на месте)
- Освобождение невалидного указателя молча игнорируется
- Глобальный мьютекс — масштабирование по потокам ограничено
- Linux-only (`sbrk`, `LD_PRELOAD`)

## 📚 Референсы

- [Memory Allocators 101 — write a simple memory allocator](https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory) — простой линейный аллокатор и интеграция в Linux
- [Аллокаторы памяти: как это работает](https://habr.com/ru/articles/435698/) — концепция аллокаторов
