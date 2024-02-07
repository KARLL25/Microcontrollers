
const int row_count_revised = 2; 
const int column_count_revised = 2; 
const int keydown_revised = 1; 
const int keyhold_revised = 3; 
const int keyup_revised = 2; 
const int keyfree_revised = 0; 
int row_pins_revised[] = {A4, A3}; 
int column_pins_revised[] = {2, 3}; 

int key_status_revised[row_count_revised][column_count_revised] = { }; 
String key_map_revised[row_count_revised][column_count_revised] = { 
    { "Row 1, Col 1", "Row 1, Col 2" },
    { "Row 2, Col 1", "Row 2, Col 2" }
};

void setup() {
  
  Serial.begin(9600);
  
  for (int i = 0; i < row_count_revised; i++) { 
    for (int j = 0; j < column_count_revised; j++) { 
      key_status_revised[i][j] = keyfree_revised; 
    }
  }
  
  for (int i = 0; i < column_count_revised; i++) { 
    pinMode(column_pins_revised[i], INPUT_PULLUP); 
  }
  
  for (int i = 0; i < column_count_revised; i++) { 
    pinMode(row_pins_revised[i], INPUT); 
  }
}

void loop() {
  for (int i = 0; i < row_count_revised; i++) { 
    pinMode(row_pins_revised[i], OUTPUT); 
    digitalWrite(row_pins_revised[i], LOW); 
    for (int j = 0; j < column_count_revised; j++) { 
      int sensor_val = digitalRead(column_pins_revised[j]); 
      if (sensor_val == LOW && key_status_revised[i][j] == keyfree_revised) { 
        Serial.println(key_map_revised[i][j]);  
        Serial.println("KEYDOWN_REVISED"); 
        key_status_revised[i][j] = keydown_revised; 
      } if (sensor_val == LOW && key_status_revised[i][j] == keydown_revised) { 
        Serial.println(key_map_revised[i][j]);  
        Serial.println("KEYHOLD_REVISED"); 
        key_status_revised[i][j] = keyhold_revised; 
      } else if (sensor_val == HIGH && (key_status_revised[i][j] == keydown_revised || key_status_revised[i][j] == keyhold_revised)) { 
        key_status_revised[i][j] = keyup_revised; 
        Serial.println(key_map_revised[i][j]);  
        Serial.println("KEYUP_REVISED"); 
      } else if (sensor_val == HIGH && key_status_revised[i][j] == keyup_revised) { 
        key_status_revised[i][j] = keyfree_revised; 
        Serial.println(key_map_revised[i][j]);  
        Serial.println("KEYFREE_REVISED"); 
      } 
    }
    pinMode(row_pins_revised[i], INPUT); 
  }
}
