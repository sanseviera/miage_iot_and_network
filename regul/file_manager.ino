
void  listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\r\n", dirname);
  File root = fs.open(dirname);
  if(!root){
    Serial.println("- failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print(" DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else
    {
      Serial.print(" FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file= root.openNextFile();
    }
}

 char* readFile(fs::FS&fs,const char* path){
  char* tmp = (char*)malloc(sizeof(char) * 1024); 
  File file= fs.open(path);
  if(!file || file.isDirectory()){
      Serial.println("- failed to open file for reading");
      return (char*)"Erreur de lecture";
  }
  Serial.println("- read from file:");
  int index = 0;
  while(file.available()){
    //Serial.write(file.read());
    tmp[index++] = (char)file.read();
  }
  file.close();
  return (char*)tmp;
}

void writeFile(fs::FS&fs, const char* path, const char* message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS&fs, const char* path, const char* message){
  Serial.printf("Appending to file: %s\r\n", path);
  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("- failed to open file for appending");
    return;
  }
  if(file.print(message)){
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

void renameFile(fs::FS&fs, const char* path1, const char* path2){
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("- file renamed");
  } else {
  Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS&fs, const char* path){
  Serial.printf("Deleting file: %s\r\n", path);
  if(fs.remove(path)){
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

void verifFile()
{
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
}  
}
