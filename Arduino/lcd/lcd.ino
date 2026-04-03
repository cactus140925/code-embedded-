#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>

LiquidCrystal lcd(13, 12, 7, 6, 5, 4);
Servo dongco;

const byte ROWS = 4;
const byte COLS = 4;
#define IR A1
// Mảng chứa các ký tự trên bàn phím ma trận
char hexaKeys[ROWS][COLS] = {
  {'7', '8', '9', '/'},
  {'4', '5', '6', '*'},
  {'1', '2', '3', '-'},
  {'C', '0', '=', '+'}
};

// Khai báo các chân cho hàng và cột
byte pinrow[ROWS] = {11,10,9,8};
byte pincol[COLS] = {3, 2, 0, A0};

// Khởi tạo đối tượng Keypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), pinrow, pincol, ROWS, COLS);

char password[5] = {'1', '2', '3', '4', '*'}; // Mật khẩu cần nhập
char input[5]; // Mảng lưu ký tự nhập vào
int index = 0; // Vị trí hiện tại trong mảng input

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  dongco.attach(A3);
  lcd.setCursor(0, 0);
  lcd.print("Nhap mat khau:");
  dongco.write(0); // Khóa cửa (góc 0 độ)
  pinMode(IR,INPUT);
}

void loop() {
  bool check = digitalRead(IR);
  if(check== HIGH)
  {
  char key = customKeypad.getKey(); // Đọc phím bấm

  if (key) { // Kiểm tra nếu có phím được nhấn
    if (key == 'C') { // Nếu nhấn phím 'C', xóa nhập liệu
      lcd.clear();
      index = 0; // Đặt lại vị trí index
      lcd.setCursor(0, 0);
      lcd.print("Nhap mat khau:");
    } else if (index < 5) { // Thu thập tối đa 5 ký tự
      input[index] = key;
      lcd.setCursor(index, 1);
      lcd.print(key);
      index++;

      if (index == 5) { // Khi đã nhập đủ 5 ký tự
        if (strncmp(password, input, 5) == 0) { // So sánh mật khẩu
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Mat khau dung");
          lcd.setCursor(0, 1);
          lcd.print("Cua mo");
          dongco.write(90);
          delay(2000);
          lcd.clear();
          for(int i=10;i>0;i--)
          {
            lcd.setCursor(0,0);
            lcd.print("cua se dong sau :");
            lcd.setCursor(0,1);
            lcd.print(i);
            delay(900);
            lcd.clear();
          }
          dongco.write(0);
          lcd.setCursor(0,0);
          lcd.print("nhap mat khau:");
          index=0;
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Mat khau sai");
          delay(5000); 
          lcd.clear();
          index = 0; // Đặt lại vị trí  để nhập lại
          lcd.setCursor(0, 0);
          lcd.print("Nhap mat khau:");
        }
      }
    }
  }
}
}
