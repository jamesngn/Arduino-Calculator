#include <LiquidCrystal.h>
#include <Keypad.h>

LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', '+' },
  { '4', '5', '6', '-' },
  { '7', '8', '9', 'x' },
  { 'C', '0', '=', '.' }
};

uint8_t colPins[COLS] = { 30, 32, 34, 36 }; // Pins connected to C1, C2, C3, C4
uint8_t rowPins[ROWS] = { 22, 24, 26, 28 }; // Pins connected to R1, R2, R3, R4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//declare variables
int cursorColumn = 0;
char keyNumbs[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
float number = 0;

float inputNumbers[20];
int inputLength = 0;

char inputOperators[19];

bool isOn = true;
bool isFloatingMode = false;
int floatNo = 0;

char preKey = ' ';
int AdjacentOperatorsMode = 0;

int minusN = 0;

bool error = false;
String error_msg;

bool finished = false;

uint8_t cursor[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b00000,
};

void showSplashScreen(String stationary, String moving) {
  lcd.print(stationary);
  lcd.setCursor(2, 1);
  String message = moving;
  for (byte i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    delay(75);
  }
  delay(500);
}



void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  showSplashScreen("Swin Calculator", "By Quang & Tu");
  lcd.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
  char key = keypad.getKey();
  if (key) {
    if (!finished) {
      lcd.setCursor(cursorColumn, 0);
      lcd.println(key);
      cursorColumn++;

      ProcessInputNumberAndOperator(key);
      preKey = key;

      if (key == '=' && !error) {

        printArray<float>(inputNumbers, inputLength);
        printArray<char>(inputOperators, inputLength - 1);
        for (int i = 0; i < inputLength - 1; i++) {
          if (inputOperators[i] == 'x') {
            MergeTwoElementsByOperator(i, inputOperators[i]);
            RemoveElementInArrayWithIndex<char>(/*array*/inputOperators,/*size*/inputLength,/*removedIindex*/i);
            printArray<float>(inputNumbers, inputLength);
            printArray<char>(inputOperators, inputLength - 1);
            i = 0;
          }
        }

        while (inputLength > 1)  {
          MergeTwoElementsByOperator(0, inputOperators[0]);
          RemoveElementInArrayWithIndex<char>(/*array*/inputOperators,/*size*/inputLength,/*removedIndex*/0);
          printArray(inputNumbers, inputLength);
          printArray(inputOperators, inputLength - 1);
        }

        lcd.setCursor(0, 1);
        lcd.println(inputNumbers[0]);
      }
      else if (key == '=' && error) {
        lcd.setCursor(0, 3);
        lcd.println(error_msg);
      }
    }
    if (key == 'C' && isOn) {
      TurnOffLCD();
    }
    else if (key == 'C' && !isOn) {
      TurnOnLCD();
    }
  }
}
void MergeTwoElementsByOperator(int startIndex, char op) {
  float newElement = ProcessResult(inputNumbers[startIndex], inputNumbers[startIndex + 1], op);
  inputNumbers[startIndex] = newElement;
  RemoveElementInArrayWithIndex(inputNumbers, inputLength, startIndex + 1);
  inputLength--;
}
float ProcessResult(float number1, float number2, char operation) {
  float result = 1;
  switch (operation) {
    case '+':
      result = number1 + number2;
      break;
    case '-':
      result = number1 - number2;
      break;
    case 'x':
      result = number1 * number2;
      break;
    case '/':
      result = number1 / number2;
      break;
    default:
      break;
  }
  return result;
}
void ProcessInputNumberAndOperator(char keyUserInput) {
  ProcessInputNumber(keyUserInput);
  ProcessOperator(keyUserInput);
}
void ProcessInputNumber(char keyUserInput) {
  for (int i = 0; i <= 9; i++) {
    if (keyNumbs[i] == keyUserInput) { //check the input with the key number array
      minusN = 0;
      if (number == 0.0 && !isFloatingMode) { //check whether the number is initialised or not
        number = i;
      }
      else if (isFloatingMode) {
        floatNo++;
        number += i / (pow(10, floatNo));
      }
      else if (number != 0.0 && !isFloatingMode) {
        number = number * 10 + i;
      }
    }
  }
}
void ProcessOperator(char keyUserInput) {
  if ((keyUserInput == '+' || keyUserInput == '-' || keyUserInput == 'x' || keyUserInput == '/')) {
    //Add the new operator to the array;
    inputOperators[inputLength] = keyUserInput;
    if (!isPrekeyOperator()) {
      AddToArray();
      ResetKeyVariables();
    }
    else if (isPrekeyOperator && keyUserInput == '-') {
      minusN++;
      Serial.print("minusN: ");
      Serial.println(minusN);
    }
    changeAdjacentOperatorsMode(keyUserInput);
  }
  else if (keyUserInput == '.' ) {
    if (!isFloatingMode) {
      isFloatingMode = true;
    }
    else {
      error = true;
      error_msg = "SYNTAX ERROR";
    }
  }
  else if (keyUserInput == '=') {
    changeAdjacentOperatorsMode(keyUserInput);
    if (!error) {
      AddToArray();
      ResetKeyVariables();
    }
    finished = true;
  }
}
bool isPrekeyOperator() {
  return (preKey == '+' || preKey == '-' || preKey == 'x' || preKey == '/');
}
void changeAdjacentOperatorsMode(char currKey) {
  if (currKey != '=') {
    if (minusN % 2 == 1) {
      AdjacentOperatorsMode = 1;
    }
    else if (minusN % 2 == 0) {
      AdjacentOperatorsMode = 2;
    }
  }

  //cover syntax error: 1+x2 ...
  if ((currKey == 'x' || currKey == '=') && (preKey == '+' ||  preKey == '-' || preKey == 'x')) {
    AdjacentOperatorsMode = 3;
    error = true;
    error_msg = "SYNTAX ERROR";
  }

  Serial.print("AdjacentOperatorsMode: ");
  Serial.println(AdjacentOperatorsMode);
}
void AddToArray() {
  switch (AdjacentOperatorsMode) {
    case 0: case 2:
      inputNumbers[inputLength] = number;
      break;
    case 1:
      inputNumbers[inputLength] = number * -1;
      break;
    case 3:
      Serial.println("SYNTAX ERROR");
      break;
    default:
      break;
  }
  Serial.print(inputNumbers[inputLength]);
  Serial.println(" is stored to the array");
  inputLength++;
}
void ResetKeyVariables() {
  number = 0;
  isFloatingMode = false;
  floatNo = 0;
  AdjacentOperatorsMode = 0;
}
void ResetAll() {
  ResetKeyVariables();
  error = false;
  lcd.clear();
  cursorColumn = 0;
  ZeroArray(inputNumbers, 20);
  inputLength = 0;
  error_msg = "";
}
void TurnOnLCD() {
  lcd.display();
  lcd.clear();
  showSplashScreen("Swin Calculator", "By Quang & Tu");
  ResetAll();
  finished = false;
  isOn = true;
}
void TurnOffLCD() {
  lcd.clear();
  showSplashScreen("Swin Calculator", "Goodbye !!!!");
  lcd.noDisplay();
  ResetAll();
  isOn = false;
}

template<typename Value>
void RemoveElementInArrayWithIndex (Value arr[], int size, int removedIndex) {
  for (int i = removedIndex; i < inputLength; i++) {
    arr[i] = arr[i + 1];
  }
}
void ZeroArray(float arr[], int size) {
  for (int i = 0; i < size; i++) {
    arr[i] = 0;
  }
}
//use print function for debugging only
template<typename Value>
void printArray(Value arr[], int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(arr[i]);
    Serial.print(" ");
  }
  Serial.println();
}
