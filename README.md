
Using pragma back to align padding of strucs

Pmagic function reutns magic bytes in order
detect_file_type returns fily type along with magic bytes

BMP_header does header analysis of the file (BMP)
Extract_text prints readable ascii (32 to 126)
Hex dump prints bonary data in hex form of 2 hexadecimal numbers at a time
with padding and serial numbers.


BMP function passes pixel data of a BMP image
Lsb_analysis collects pixel data and strips the LSB’s from pixeldata.
Decode_lsb reconstructs these bits and saves them as ascii.

Encode_lsb does all above steps but for encoding.

Read_message function “reads” the text stored in a txt file and returns it
Report generation collects the new output and prints it onto an external file.


In main,
proper CLI integration for each function.
each function can be independently called by the user.
Calls each function in order accordingly.

 
Faults:
incorrect memory handling for LSB_encoding function
lack of custom stegnography.
