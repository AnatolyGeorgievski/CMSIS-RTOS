#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

//#define CONFIG_OFFSET (2112*50) // должно быть выравнивание на размер страницы стирания памяти
#define CONFIG_ATTR     __attribute__((used, section("CONFIG")))

void FLASH_LoadConfig();
#endif // CONFIG_H_INCLUDED
