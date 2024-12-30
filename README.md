I regularly analyze infection chains that use multiple stages in which a shellcode is passed as a parameter to a new thread. This is an attempt to replicate that mechanism.  
This shellcode loader:  
* Opens an encrypted shellcode from a file named database.dat
* It creates a new suspended thread and passes the shellcode and its size as parameters (in a struct of type PAYLOAD_INFO)
* It sleeps for 15 seconds and then decrypts the shellcode
* It resumes the thread, loading the shellcode in memory. The shellcode is benign and simply runs the calculator.
