#include <fstream>
#include <iostream>
#include <ctime>

class SimpleLogger
{
private:
    const char* filename; // Имя файла для логирования
    std::ofstream logFile;

    // Функция для получения текущего времени
    const char* getCurrentTime( )
    {
        time_t now = time( 0 );
        return ctime( &now ); // Получаем строку времени в формате "Wed Jun 30 21:49:08 2021\n"
    }

public:
    // Конструктор принимает имя файла и открывает его
    SimpleLogger( const char* logFilename )
        : filename( logFilename )
    {
        logFile.open( filename, std::ios::out | std::ios::app );
        if ( !logFile.is_open( ) )
        {
            std::cerr << "Ошибка при открытии файла для записи логов!" << std::endl;
        }
    }

    // Деструктор закрывает файл
    ~SimpleLogger( )
    {
        if ( logFile.is_open( ) )
        {
            logFile.close( );
        }
    }

    // Функция для записи сообщения в лог
    void log( const char* message )
    {
        if ( logFile.is_open( ) )
        {
            logFile << "[" << getCurrentTime( ) << "] " << message << std::endl;
        }
    }
};
