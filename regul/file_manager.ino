/*
 * Permet d'afficher sur le port serie la liste des fichier d'un dossier spécifié à l aide d un chemin
 */
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

/*
 * Permet d'afficher sur le port serie le contenu d'un fichier  spécifié à l aide d un chemin
 */
char* readFile(fs::FS& fs, const char* path) {
    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return NULL; // Retourne NULL pour indiquer une erreur
    }

    // Obtenir la taille du fichier
    size_t fileSize = file.size();

    // Allouer un tampon de la taille du fichier plus un caractère nul de terminaison
    char* tmp = (char*)malloc(fileSize + 1);
    if (!tmp) {
        Serial.println("- failed to allocate memory");
        file.close();
        return NULL;
    }

    Serial.println("- reading from file:");
    size_t index = 0;
    while (file.available()) {
        tmp[index++] = (char)file.read();
    }
    tmp[index] = '\0'; // Terminer la chaîne avec un caractère nul

    file.close();
    return tmp;
}

/*
 * Permet d'écrir un fichier spécifié à l aide d un chemin spécifié et du contenu
 */
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

/*
 * Permet d'd'ajouter dans un fichier spécifié à l aide d un chemin spécifié et du contenu
 */
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

/*
 * Permet de renomer un fichier spécifié avec un chemin
 */
void renameFile(fs::FS&fs, const char* path1, const char* path2){
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("- file renamed");
  } else {
  Serial.println("- rename failed");
  }
}
/*
 * Permet de supprimer un fichier spécifié avec un chemin
 */
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
    return;
}  
}
