# README  
## Updates  
#### **6:43 AM 1/26/2025**  
- Had to uninstall/install the platform "Espressif 32"  
  - dependencies were missing and would not compile.  

#### **5:22 AM 1/26/2025**  
- This is being updated from the laptop. Apparently, PIO has  
  changed in the 4+ years since this project was created.  

- Workflow now suggests not using global libraries shared with  
  all projects, but adding them to the project.  

- **Libraries are added in the .PIO directory.**   
  - EX: .pio\libdeps\esp12e\
  - EX: .pio\libdeps\esp12e\RTClib
  - EX: .pio\libdeps\esp12e\integrity.dat
  - This project still has the old /lib and /include dirs with the README files.  