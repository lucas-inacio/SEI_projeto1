#ifndef COMMON_HPP
#define COMMON_HPP

#define LED1 19
#define LED2 21

// Indica se alterações devem ser aplicadas e o dispositivo reiniciado
#ifndef IMPLEMENTATION
extern volatile bool aplicarConfig;
extern volatile float umidade;
extern volatile float temperatura; 
#endif

#endif // COMMON_HPP