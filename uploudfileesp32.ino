#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Pasti Dadi";
const char *password = "8888888888";
const int ledPin = 13;  // Change to the pin where you connect the LED


AsyncWebServer server(80);

String getFileExtension(String filename) {
  int dotIndex = filename.lastIndexOf('.');
  if (dotIndex >= 0) {
    return filename.substring(dotIndex + 1);
  }
  return "";  // Return an empty string if no extension found
}

String getFileType(String fileExtension) {
  if (fileExtension.equalsIgnoreCase("pdf") || fileExtension.equalsIgnoreCase("pptx") || fileExtension.equalsIgnoreCase("docx") || fileExtension.equalsIgnoreCase("xlsx")){
    return "Document";
  } else if (fileExtension.equalsIgnoreCase("jpg") || fileExtension.equalsIgnoreCase("jpeg") || fileExtension.equalsIgnoreCase("png")) {
    return "Picture";
  }else if (fileExtension.equalsIgnoreCase("mp3")) {
    return "Music";
  }
  return "Unknown";
}

String generateUniqueFilename(String fileType, String originalFilename, int fileNumber) {
  String fileExtension = getFileExtension(originalFilename);
  String uniqueFilename;

  if (fileNumber > 0) {
    uniqueFilename = originalFilename.substring(0, originalFilename.lastIndexOf('.')) + "_" + fileNumber + "." + fileExtension;
  } else {
    uniqueFilename = originalFilename;
  }

  return fileType + "/" + uniqueFilename;
}

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  static File file;
  static int fileNumber = 1;  // Start from 1

  if (!index) {
    String fileExtension = getFileExtension(filename);
    String fileType = getFileType(fileExtension);
    String uniqueFilename = generateUniqueFilename(fileType, filename, fileNumber);
    String filePath = "/" + uniqueFilename;

    while (SD.exists(filePath)) {
      // If file already exists, increment the file number
      fileNumber++;
      uniqueFilename = generateUniqueFilename(fileType, filename, fileNumber);
      filePath = "/" + uniqueFilename;
      Serial.println("nama file patch:" + filePath);
    }

    file = SD.open(filePath, FILE_WRITE);
    if (!file) {
      Serial.println("Gagal membuka file");
      return request->send(500, "text/plain", "Gagal membuka file");
    }
  }

  if (file.write(data, len) != len) {
    Serial.println("Gagal menulis ke file");
    return request->send(500, "text/plain", "Gagal menulis ke file");
  }

  if (final) {
    file.close();
    Serial.println("File berhasil disimpan di SD card");
    // Activate LED for visual feedback
    digitalWrite(ledPin, HIGH);  // Turn on the LED
    delay(500);                   // Delay for 500ms (or adjust as needed)
    digitalWrite(ledPin, LOW);   // Turn off the LED
    fileNumber = 1;         // Reset fileNumber for the next upload
  }
}

void setup() {
  Serial.begin(115200);

  // Menghubungkan ke Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("Terhubung ke Wi-Fi");
  Serial.print("Alamat IP server: ");
  Serial.println(WiFi.localIP());

  // Menginisialisasi SD card
  if (!SD.begin(SS)) {
    Serial.println("Gagal menginisialisasi SD card");
    return;
  }
 pinMode(ledPin, OUTPUT);

  // Periksa dan buat direktori jika belum ada
  if (!SD.exists("Document")) {
    SD.mkdir("Document");
  }
  if (!SD.exists("Picture")) {
    SD.mkdir("Picture");
  }
  if (!SD.exists("Music")) {
    SD.mkdir("Music");
  }

  // Menetapkan handler untuk menerima file
  server.on(
    "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "File berhasil diunggah");
    },
    handleFileUpload);

  server.begin();
}

void loop() {
  // Loop kosong
}
