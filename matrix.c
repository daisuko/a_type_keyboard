#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"

#ifndef DEBOUNCE
#  define DEBOUNCE  5
#endif

static uint8_t debouncing = DEBOUNCE;

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

static matrix_row_t read_cols(void);
static void init_cols(void);
static void unselect_rows(void);
static void select_row(uint8_t row);

#define LED_ON()  do { DDRC |=  (1<<5); PORTC |=  (1<<5); } while (0)
#define LED_OFF() do { DDRC &= ~(1<<5); PORTC &= ~(1<<5); } while (0)

void matrix_init(void)
{
  // initialize row and col
  unselect_rows();
  init_cols();

  // initialize matrix state: all keys off
  for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
    matrix[i] = 0;
    matrix_debouncing[i] = 0;
  }

  LED_ON();
  _delay_ms(500);
  LED_OFF();
}

uint8_t matrix_scan(void)
{
  for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
    select_row(i);
    _delay_us(30);  // without this wait read unstable value.
    matrix_row_t cols = read_cols();

    if (matrix_debouncing[i] != cols) {
      matrix_debouncing[i] = cols;

      if (debouncing) {
        debug("bounce!: "); debug_hex(debouncing); debug("\n");
      }

      debouncing = DEBOUNCE;
    }

    unselect_rows();
  }

  if (debouncing) {
    if (--debouncing) {
      _delay_ms(1);
    } else {
      for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = matrix_debouncing[i];
      }
    }
  }

  return 1;
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
  return matrix[row];
}

/* Column pin configuration
 * col: 0   1   2   3   4   5   6   7
 * pin: B0  B1  B2  B3  B4  B5  B6  B7
 */
static void  init_cols(void)
{
  // Input with pull-up(DDR:0, PORT:1)
  DDRB  &= ~0b11111111;
  PORTB |=  0b11111111;
}

static matrix_row_t read_cols(void)
{
  return ~PINB;
}

/* Row pin configuration
 * row: 0   1   2   3   4   5   6   7
 * pin: D0  D1  D2  D3  D4  D5  D6  C2
 */
static void unselect_rows(void)
{
  // Hi-Z(DDR:0, PORT:0) to unselect
  DDRD  &= ~0b01111111;
  PORTD &= ~0b01111111;
  DDRC  &= ~0b00000100;
  PORTC &= ~0b00000100;
}

static void select_row(uint8_t row)
{
  // Output low(DDR:1, PORT:0) to select
  switch (row) {
    case 0:
      DDRD  |=  (1<<0);
      PORTD &= ~(1<<0);
      break;
    case 1:
      DDRD  |=  (1<<1);
      PORTD &= ~(1<<1);
      break;
    case 2:
      DDRD  |=  (1<<2);
      PORTD &= ~(1<<2);
      break;
    case 3:
      DDRD  |=  (1<<3);
      PORTD &= ~(1<<3);
      break;
    case 4:
      DDRD  |=  (1<<4);
      PORTD &= ~(1<<4);
      break;
    case 5:
      DDRD  |=  (1<<5);
      PORTD &= ~(1<<5);
      break;
    case 6:
      DDRD  |=  (1<<6);
      PORTD &= ~(1<<6);
      break;
    case 7:
      DDRC  |=  (1<<2);
      PORTC &= ~(1<<2);
      break;
  }
}
