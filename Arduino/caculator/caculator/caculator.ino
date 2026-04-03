#include <Keypad.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

const byte ROWS = 4;
const byte COLS = 4;

// Mảng chứa các ký tự trên bàn phím ma trận
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', '/'},
  {'4', '5', '6', '*'},
  {'7', '8', '9', '-'},
  {'C', '0', '=', '+'}
};

byte pinrow[ROWS] = {13, 10, 9, 8};
byte pincol[COLS] = {3, 2, A1, A0};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), pinrow, pincol, ROWS, COLS);

char dau; // Lưu dấu phép toán

float tinhso() {
  float so = 0;
  int i = 0;
  char key;
  
  while (i < 5) {
    key = customKeypad.getKey(); // Đọc phím từ bàn phím
    if (key) { // Nếu có phím được nhấn
      if (key == 'C') {
        return -1; // Nếu nhấn 'C', trả về 0
      } else if (key == '/' || key == '*' || key == '+' || key == '-'||key =='=') {
        dau = key; // Lưu dấu phép toán
        return so; // Trả về số đã nhập
      } else if (key >= '0' && key <= '9') {
        lcd.print(key); // Hiển thị ký tự lên LCD
        so = so * 10 + (key - '0'); // Cập nhật giá trị số
        i++;
      }
    }
  }
  return so;

}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Khoi dong");
  delay(2000);
  lcd.clear();
}

void loop() {
  int check=0;
  float a = tinhso(); // Đọc số thứ nhất
  if (a == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nhap lai:");
    return;
  } else {
    if(dau=='=')
    {
      check=1;
    }else
    {
    lcd.setCursor(0, 1);
    lcd.print(dau); 
    } 
  }
 if(check==0)
 {
   char check1= dau;
  float b = tinhso(); 
if(b==-1)
{
  lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nhap lai:");
    return;
}else{

  // Thực hiện phép toán
  float ketqua = 0;
  if (check1 == '+') {
    ketqua = a + b;
  } else if (check1 == '-') {
    ketqua = a - b;
  } else if (check1 == '*') {
    ketqua = a * b;
  } else if (check1 == '/') {
    if (b != 0) {
      ketqua = a / b;
    } else {
      lcd.clear();
      lcd.print("Loi: Chia 0");
      return;
    }
  }

  // Hiển thị kết quả
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ket qua:");
  lcd.setCursor(0, 1);
  lcd.print(ketqua);
}
}else
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ket qua:");
  lcd.setCursor(0,1);
  lcd.print(a);
}
while(1)
{
    char getout = customKeypad.getKey(); 
    if(getout=='C')
    {
      lcd.clear();
      break;
    }
}
}
