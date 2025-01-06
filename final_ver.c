//TODO: lsb, custom steg encoding/decoding

#include<stdio.h>//fopen,fread,fprintf,size_t
#include<stdlib.h>//for size_t
#include<string.h>//strerror
#include<errno.h>//errno
#include<stdint.h>//for header analysis

#define MAX_TEXT_SIZE 10000
int height,width;

#pragma pack(push,1)
//FIleheader
typedef struct{
    unsigned short type;//magic numebr
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
}BMPHeader;

//Bitmap info header
//device indipendent bitmap
typedef struct{
    unsigned int size;//usually 40 bytes
    int width;
    int height;
    unsigned short planes;//Number of colour planes
    unsigned short bitsPerPixel;//24 for colour
    unsigned int compression;//0 for no compression
    unsigned int imageSize;
    //in pixel per meter
    int xResolution;//horizontal
    int yResolution;//vertical
    unsigned int colorsUsed;//No of colours used but 0 for full color
    unsigned int importantColors;//Number of important colors
}DIBHeader;
#pragma pack(pop)

//Funct1: Magic bytes
char *pmagic(unsigned char magic[12], int a,int b){
    int i;
    char temp[16]="";
    char *result=(char *)malloc(100*sizeof(char));
    if(result==NULL){
        printf("Memory allocation failed\n");
        return NULL;
    }
    result[0]='\0';
    for(i=a; i<=b; i++){
        sprintf(temp,"%02X ",magic[i]);
        strcat(result,temp);
        temp[0]='\0';
    };
    return result;
}
char *detect_file_type(const char *filename, int p){
    FILE *file=fopen(filename,"rb");
    //FILE is a data type
    if(!file){
        fprintf(stderr,"Failed to open file: %s\n",strerror(errno));
        //perror("Failed to open file");
        return NULL;
    };

    unsigned char magic[12]={0};
    size_t a=fread(magic,1,12,file);
    //sizeof(unsigned char)=1
    //use %zu to print size_t
    fclose(file);

    if(a<8){
        fprintf(stderr, "Error: File is too small to analyze magic bytes.\n");
        return NULL;
    };

    char *magics=calloc(100,sizeof(char));
    if (!magics){
        fprintf(stderr, "Memory allocation failed.\nError: %s\n", strerror(errno));
        return NULL;
    };
    
    char temp[100]="";
    if(magic[0]==0xFF && magic[1]==0xD8 && magic[2]==0xFF){
        sprintf(temp, "\n%s\n", pmagic(magic,0,2));
        strcat(magics,"Magic Bytes: ");
        strcat(magics,temp);
        strcat(magics, "\nFile Type: JPEG\n");
        if(p==0){sprintf(magics,"JPEG");};}
    else if(magic[0]==0x89 &&magic[1]==0x50 &&magic[2]==0x4E &&magic[3]==0x47 &&magic[4]==0x0D &&magic[5]==0x0A &&magic[6]==0x1A &&magic[7]==0x0A){
        sprintf(temp, "\n%s\n", pmagic(magic,0,7));
        strcat(magics,"Magic Bytes: ");
        strcat(magics,temp);
        strcat(magics, "\nFile Type: PNG\n");
        if(p==0){sprintf(magics,"PNG");};}
    else if(magic[0]==0x47 &&magic[1]==0x49 &&magic[2]==0x46 &&magic[3]==0x38 && (magic[4]==0x39 || magic[4]==0x37) &&magic[5]==0x61){
        sprintf(temp, "\n%s\n", pmagic(magic,0,5));
        strcat(magics,"Magic Bytes: ");
        strcat(magics,temp);
        strcat(magics, "\nFile Type: GIF\n");
        if(p==0){sprintf(magics,"GIF");};}
    else if(magic[0]==0x42 &&magic[1]==0x4D){
        sprintf(temp, "%s\n", pmagic(magic,0,1));
        strcat(magics,"\nMagic Bytes: ");
        strcat(magics,temp);
        strcat(magics, "File Type: BMP");
        if(p==0){sprintf(magics,"BMP");};}
    else if((magic[0]==0x49 &&magic[1]==0x49 &&magic[2]==0x2A &&magic[3]==0x00) ||(magic[0]==0x4D &&magic[1]==0x4D &&magic[2]==0x00 &&magic[3]==0x2A)){
        sprintf(temp, "\n%s\n", pmagic(magic,0,3));
        strcat(magics,"\nMagic Bytes: \n");
        strcat(magics,temp);
        strcat(magics, "File Type: TIFF");
        if(p==0){sprintf(magics,"TIFF");};}
    else if(magic[0]==0x52 &&magic[1]==0x49 &&magic[2]==0x46 &&magic[3]==0x46 &&magic[8]==0x57 &&magic[9]==0x45 &&magic[10]==0x42 &&magic[11]==0x50){
        //WEBP reads 0,1,2,3,8,9,10,11 magic bytes
        strcat(magics,"\nMagic Bytes: \n");
        sprintf(temp, "\n%s\n", pmagic(magic,0,3));
        strcat(magics,temp);
        sprintf(temp, "\n%s\n", pmagic(magic,8,11));        
        strcat(magics,temp);
        strcat(magics, "File Type: WEBP");
        if(p==0){sprintf(magics,"WEBP");};}
    else if(magic[0]==0x00 &&magic[1]==0x00 &&magic[2]==0x01 &&magic[3]==0x00){
        sprintf(temp, "\n%s\n", pmagic(magic,0,3));
        strcat(magics,"\nMagic Bytes: \n");
        strcat(magics,temp);
        strcat(magics, "File Type: ICO");
        if(p==0){sprintf(magics,"ICO");};}
    else if(magic[4]==0x66 &&magic[5]==0x74 &&magic[6]==0x79 &&magic[7]==0x70 && (magic[8]==0x68 &&magic[9]==0x65 &&magic[10]==0x69 &&magic[11]==0x63)){
        //HEIC/HEIF reads 4,5,6,7,8,9,10,11 magic bytes
        sprintf(temp, "\n%s\n", pmagic(magic,4,11));
        strcat(magics,"\nMagic Bytes: \n");
        strcat(magics,temp);
        strcat(magics, "File Type: HEIC/HEIF");
        if(p==0){sprintf(magics,"HEIC/HEIF");};}
    else{
        sprintf(temp, "\n%s\n", pmagic(magic,0,7));
        strcat(magics,"\nMagic Bytes: \n");
        strcat(magics,temp);
        strcat(magics, "File Type: Unknown");
        if(p==0){sprintf(magics,"Unknown");};}
    return magics;
}

//header
void BMP_header(const char *filename, char *out){
    FILE *file=fopen(filename, "rb");
    if(!file){
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }

    unsigned char header[54];
    if(fread(header,1,54,file)!=54){
        fprintf(stderr, "Failed to read BMP header.\n");
        fclose(file);
        return;
    }

    //Check if the file is a BMP
    if(header[0]!='B'||header[1]!='M'){
        fprintf(stderr, "Not a BMP file.\n");
        fclose(file);
        return;
    }

    uint32_t fileSize=*(uint32_t *)&header[2];
    uint32_t compression=*(uint32_t *)&header[30];
    uint32_t colorSpace=*(uint32_t *)&header[46];
    //uint32_t is 4 bytes


    sprintf(out, "\n24-bit BMP File Analysis:\nFile Type: BM (Valid BMP)\nFile Size: %u bytes\nCompression Method: ", fileSize);
    char temp[50];


    if(compression==0){
        strcat(out,"None (BI_RGB)\n");
    }else if(compression==1){
        strcat(out,"RLE-8(Run-Length Encoding for 8-bit data)\n");
    }else if(compression==2){
        strcat(out, "RLE-4 (Run-Length Encoding for 4-bit data)\n");
    }else{
        sprintf(temp,"Unknown (%u)\n", compression);
        strcat(out, temp);
    }


    //Color Space
    strcat(out, "Colour Space: ");
    if(colorSpace==0){
        strcat(out,"Default sRGB\n");
    }else if(colorSpace==0x73524742){
        strcat(out,"sRGB(Standard RGB)\n");
    }else{
        sprintf(temp,"Custom Color Profile (0x%X)\n", colorSpace);
        strcat(out, temp);
    }

    fclose(file);
}

//Funct2: Readable ASCII
char *extract_text(const char *filename){
    FILE *file=fopen(filename, "rb");
    if(!file){
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        //perror("Failed to open file");
        return NULL;
    };

    char *buffer=calloc(MAX_TEXT_SIZE,sizeof(unsigned char));
    if(!buffer){
        fprintf(stderr, "Memory allocation failed.\nError: %s\n", strerror(errno));
        fclose(file);
        return NULL;
    };
    
    int index=0;
    unsigned char ch;
    while(fread(&ch, 1, 1, file) && index<MAX_TEXT_SIZE-1){
        if(ch>=32 && ch<=126){buffer[index++]=ch;};
    };
    buffer[index]='\0';
    fclose(file);
    return buffer;
}

//Funct3: Hex Dump
char *hex_dump(const char *filename){
    FILE *file=fopen(filename, "rb");
    if (!file){
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        //perror("Failed to open file");
        return NULL;
    };

    unsigned char buffer[16];
    size_t bytesRead;
    size_t offset=0;
    size_t s=0;
    
    //print
    int count=0;
    printf("\nHex Dump:\n");
    while((bytesRead=fread(buffer,1,sizeof(buffer), file))>0){
        //printf("%08X  :", (unsigned int)offset);
        //for(s=0; s<bytesRead; s++){printf("%02X ", buffer[s]);};
        //for(s=bytesRead;s<16;s++){printf("   ");};
        //printf("\n");
        offset=offset+bytesRead;
        count++;
    };
    //printf("%d %zu\n", count, offset);

    char *strhex=calloc(((offset/16)+1)*59,sizeof(char));//(59= 8+2 + (16)3 +1)
    if (!strhex){
        fprintf(stderr, "Memory allocation failed.\nError: %s\n", strerror(errno));
        return NULL;
    };
    int C=count;
    int per,pper;
    count=0;
    char temp[17];
    fseek(file,0,SEEK_SET);
    offset=0;
    s=0;
    printf("hex completion:");
    while((bytesRead=fread(buffer,1,sizeof(buffer), file))>0){
        sprintf(temp, "%08X: ", (unsigned int)offset);
        strcat(strhex,temp);
        for(s=0; s<bytesRead; s++){
            sprintf(temp, "%02X ", buffer[s]);
            strcat(strhex, temp);
        };
        for(s=bytesRead;s<16;s++){strcat(strhex,"   ");};
        strcat(strhex,"\n");
        offset=offset+bytesRead;

        count++;
        per=(count*100)/C;
        //if(count%1000==0){printf("\n%d\n", count);};
        if(per%10==0 && pper !=per){
            pper=per;
            printf("%d%% ", per);
        };
    };
    //printf("%d ", count);
    fclose(file);
    return strhex;
}

//lsb
unsigned char *BMP(const char *filename){

    FILE *file=fopen(filename, "rb");
    if(file==NULL){
        printf("Failed to open file: %s\n", filename);
        return NULL;
    };

    //Read BMP header
    BMPHeader bmpHeader;
    DIBHeader dibHeader;

    fread(&bmpHeader, sizeof(BMPHeader), 1, file);

    //Read DIB header
    fread(&dibHeader, sizeof(DIBHeader), 1, file);

    //remove
    //Debugging: Print header values


    //Verify the bits per pixel=24 for colour
    if(dibHeader.bitsPerPixel != 24){
        printf("This is not a 24-bit BMP file.\n");
        fclose(file);
        return NULL;
    };
    width=dibHeader.width;
    height=dibHeader.height;
    //3 bytes per for 1 pixel, row padding to ensure width is a multiple of 4 (for fast processing)
    int rowPadding=(4 -(width*3)%4)%4; 

    //Seek to the pixel data offset
    fseek(file, bmpHeader.offset, SEEK_SET);

    //Read pixel data
    unsigned char *pixelData=(unsigned char *)malloc(width * height * 3);//3 bytes per pixel (RGB)
    if(pixelData == NULL){
        printf("Memory allocation failed.\n");
        fclose(file);
        return NULL;
    };

    int y;
    for(y=height-1; y>=0; y--){ //BMP stores pixels from bottom to top
        fread(&pixelData[y * width * 3], 3, width, file);
        fseek(file, rowPadding, SEEK_CUR);//Skip padding at end of row
    };
    
    fclose(file);
    return pixelData;
}


unsigned char *lsb_analysis(const char *filename){
    
    FILE *file=fopen(filename, "rb");
    if(file==NULL){
        printf("Failed to open file: %s\n", filename);
        return NULL;
    };
    
    unsigned char *pixel;
    int y,x,p;

    //unsigned char(*filea[4])(const char *filename)={BMP};
    //if(strcmp(BMP,detect_file_type(filename))==0){pixel=filea[0](filename);}//BMP

    
    if(strcmp("BMP",detect_file_type(filename,0))==0){
        pixel=BMP(filename);
        if(pixel==NULL){
            return NULL;
        };
    }
    else{
        free(pixel);
        printf("\nLSB analysis made only for BMP files for now and not %s\n", detect_file_type(filename,0));
        return NULL;
    };
    
    unsigned char *LSB;
    LSB=(unsigned char *)malloc(width * height * 3);// 3 bytes per pixel
    if(LSB==NULL){
        printf("\nMemory allocation failed for LSB.\n");
        free(pixel);
        return NULL;
    };

    
    for(y=0; y<height; y++){
        for(x=0; x<width; x++){
            p=((y*width)+x)*3;
            LSB[p]=(pixel[p]&1);
            LSB[p+1]=(pixel[p+1]&1);
            LSB[p+2]=(pixel[p+2]&1);
        };
    };


    free(pixel);
    return LSB;
}


char *decode_lsb(unsigned char *lsbData){
    if(lsbData==NULL){
        printf("\nNo LSB data to decode.\n");
        return NULL;
    }

    int totalBits=width*height*3; // Total number of bits in the image
    int maxBytes=totalBits/8; // Convert bits to bytes
    char *decodedMessage=(char *)malloc(maxBytes+1); // +1 for null terminator

    if(decodedMessage==NULL){
        printf("\nMemory allocation failed for decoded message.\n");
        return NULL;
    }

    int bitIndex=0;
    int byteIndex=0;
    char currentByte=0;

    for(int i=0; i<totalBits; i++){
        // Get the LSB from the data
        int bit=lsbData[i] &1;

        // Place the bit into the correct position of the current byte
        currentByte |= (bit << bitIndex);

        bitIndex++;

        // If we've filled a byte, store it and reset for the next one
        if (bitIndex == 8) {
            decodedMessage[byteIndex] = currentByte;
            if (currentByte == '\0') { // Stop if we hit the null terminator
                break;
            }
            byteIndex++;
            currentByte = 0;
            bitIndex = 0;
        }
    }

    // Null-terminate the decoded string
    decodedMessage[byteIndex] = '\0';

    // Check if the message was too long to be valid
    if (bitIndex != 0 && byteIndex == maxBytes) {
        printf("\nWarning: Decoded message may be incomplete.\n");
    }

    return decodedMessage;
}

int encode_lsb(const char *imageFilename, const char *message) {
    FILE *file = fopen(imageFilename, "rb");
    if (file == NULL) {
        printf("Failed to open file: %s\n", imageFilename);
        return -1;
    }

    BMPHeader bmpHeader;
    DIBHeader dibHeader;
    int rowPadding = (4 - (width * 3) % 4) % 4;

    // Read BMP header
    fread(&bmpHeader, sizeof(BMPHeader), 1, file);
    // Read DIB header
    fread(&dibHeader, sizeof(DIBHeader), 1, file);

    // Store image dimensions
    width = dibHeader.width;
    height = dibHeader.height;

    // Read pixel data
    unsigned char *pixelData = (unsigned char *)malloc(width * height * 3);
    if (pixelData == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return -1;
    }

    fseek(file, bmpHeader.offset, SEEK_SET);
    for (int y = height - 1; y >= 0; y--) {
        fread(&pixelData[y * width * 3], 3, width, file);
        fseek(file, rowPadding, SEEK_CUR); // Skip padding at end of row
    }
    fclose(file);

    // Prepare the message to be encoded (append null terminator)
    size_t totalBits = strlen(message) * 8;

    unsigned char *encodedPixelData = (unsigned char *)malloc(width * height * 3);
    if (encodedPixelData == NULL) {
        printf("Memory allocation failed for encoded pixel data.\n");
        free(pixelData);
        return -1;
    }

    // Copy original pixel data to the encoded pixel data array
    memcpy(encodedPixelData, pixelData, width * height * 3);

    // Encode the message in the LSBs of the pixel data
    int bitIndex = 0;
    for (int i = 0; i < totalBits; i++) {
        int pixelIndex = bitIndex / 8; // Index into the pixel data
        int channel = (bitIndex % 8) / 3;  // Determine the channel (RGB)

        // Get the current bit to encode (message bit)
        int bit = (message[i / 8] >> (7 - (i % 8))) & 1;

        // Modify the LSB of the pixel channel
        unsigned char *currentPixel = &encodedPixelData[pixelIndex * 3]; // Access the RGB channels
        currentPixel[channel] &= ~1;  // Clear the LSB
        currentPixel[channel] |= bit; // Set the new bit in the LSB

        bitIndex++;
    }

    // Write the modified pixel data to a new file
    char outputFilename[256];
    snprintf(outputFilename, sizeof(outputFilename), "%s_encoded.bmp", imageFilename);

    file = fopen(outputFilename, "wb");
    if (file == NULL) {
        printf("Failed to open output file: %s\n", outputFilename);
        free(pixelData);
        free(encodedPixelData);
        return -1;
    }

    // Write BMP header and DIB header
    fwrite(&bmpHeader, sizeof(BMPHeader), 1, file);
    fwrite(&dibHeader, sizeof(DIBHeader), 1, file);

    // Write the modified pixel data
    fseek(file, bmpHeader.offset, SEEK_SET);
    for (int y = height - 1; y >= 0; y--) {
        fwrite(&encodedPixelData[y * width * 3], 3, width, file);
        fwrite("\0", rowPadding, 1, file); // Write row padding
    }

    fclose(file);

    // Cleanup
    free(pixelData);
    free(encodedPixelData);

    printf("Message successfully encoded in %s\n", outputFilename);
    return 0;
}


//read txt file
int read_message(const char *filename, char **message) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open message file: %s\n", filename);
        return -1;
    }

    // Find the length of the file
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the message + 1 for the null terminator
    *message = (char *)malloc(fileSize + 1);
    if (*message == NULL) {
        printf("Memory allocation failed for message.\n");
        fclose(file);
        return -1;
    }

    // Read the file content into the message buffer
    fread(*message, 1, fileSize, file);
    (*message)[fileSize] = '\0'; // Null-terminate the string

    fclose(file);
    return 0;
}


//report generation
void report(const char *filename, const char *message){
    char report[256];
    snprintf(report, sizeof(report), "%s_report.txt", filename);

    FILE *file = fopen(report, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    /*
    FILE *file = fopen("report.txt", "a");  // Open the file for appending
    if (file == NULL) {
        printf("Failed to open report.txt\n");
        return;  // Return if file couldn't be opened
    }
    */

    // Write the message to the file
    fputs(message, file);
    fclose(file);  // Close the file
    printf("Message appended to report.txt\n");
    return;
}


int main(int argc, char *argv[]){
    int m,t,f,x,d,e,pr;
    m=t=f=x=d=e=pr=0;
    if(argc==2 && strcmp(argv[1], "-h") == 0){
        fprintf(stderr, "Usage of %s:\n (-mtfxdp)\n-m for file type\n-t for ascii text (readable)\n-f for file header analysis\n-x for hex dump\n-d for decoding lsb text\n-p for printing the output \nor\n-e <path to .txt file to be encoded> for endcoding lsb data\n", argv[0]);
        return 0;
    }
    else if(argc==3){
        if(strchr(argv[1], 'm')){
            m=1;
        };
        if(strchr(argv[1], 't')){
            t=1;
        };
        if(strchr(argv[1], 'f')){
            f=1;
        };
        if(strchr(argv[1], 'x')){
            x=1;
        };
        if(strchr(argv[1], 'd')){
            d=1;
        };
        if(strchr(argv[1], 'p')){
            pr=1;
        };
    }
    else if(argc==4){
        if(strchr(argv[1], 'e')){
            e=1;
        };
        if(strchr(argv[1], 'p')){
            pr=1;
        };
    }
    else{
        fprintf(stderr, "\nrefer usage from: %s <filename> -h\n", argv[0]);
        return 1;
    };
    
    const char *filename = argv[2];

    char *output=(char *)malloc(100000*sizeof(char));
    if(output==NULL){
        printf("Memory allocation failed\n");
        return 1;
    };
    output[0]='\0';

    //magic bytes
    if(m==1){
        char *magic;
        printf("\nexecuting file type\n");
        magic=detect_file_type(filename,1);
        if(magic==NULL){
            free(output);
            return 1;
        };
        strcat(output,"\n");
        strcat(output,magic);
        strcat(output,"\n");
        free(magic);
        printf("file type succesful\n");
    };

    //file header
    if(f==1){
        printf("\n executnig header analysis\n");
        char header[500];
        BMP_header(filename, header);
        strcat(output,header);
        printf("header analysis successful");
    }
    
    //text extract
    if(t==1){
        char *buffera;
        printf("\nexecuting readable text extraction\n");
        buffera=extract_text(filename);
        if(buffera==NULL){
            free(output);
            return 1;
        };
        strcat(output,"\nExtracted text: \n");
        strcat(output,buffera);
        strcat(output,"\n");
        free(buffera);
        printf("readable text succesful\n");
    }

    //hex
    if(x==1){
        char *hex;
        printf("\nexecuting hex dump\n");
        hex=hex_dump(filename);
        if(hex==NULL){
            free(output);
            return 1;
        };
        strcat(output,"\nHex dump: \n");
        strcat(output,hex);
        strcat(output,"\n");
        //free(hex);
        printf("\nhex dump successful\n");
    }


    //lsb analysis
    //decode
    if(d==1){
        printf("\nexecuting lsb decoding\n");
        unsigned char *lsb;
        lsb=lsb_analysis(filename);
        if(lsb==NULL){
            free(output);
            return 1;
        };
        strcat(output,"LSB analysis: Text extracted:\n");
        char *decoded_mssg = decode_lsb(lsb);
        if(decoded_mssg==NULL){
            free(output);
            free(lsb);
            return 1;
        };
        strcat(output,decoded_mssg);
        strcat(output,"\n");
        free(lsb);
        printf("\ndeocded message is: %s", decoded_mssg);
        free(decoded_mssg);
        printf("lsb decoding successful");

    }
    
    /*
    for(int i=0; i<width*height*3; i++){
        printf("%d ", lsb[i]);//(0 or 1)
        if((i+1)%16==0){
            printf("\n");
        };
    };
    printf("\n\n");
    */

    //encode
    if(e==1){
        printf("\nexecuting LSB encoding\n");
        const char *textfile = argv[3];
        char *message=NULL;
        printf("reading text file\n");
        if(read_message(textfile,&message)!=0){
            fprintf(stderr, "Failed to read message from file: %s\n",textfile);
            free(message);
            free(output);
            return 1;
        };
        printf("%s",message);
        printf("encoding message\n");
        if(encode_lsb(filename,message)!=0){
            fprintf(stderr, "Failed to encode message into image: %s\n", filename);
            free(message);
            free(output);
            return 1;
        };
        free(message);
        printf("\nencoding message successful\n");
    };

    //report
    if(argc==3){
        printf("\ngenerating report\n");
        report(filename,output);
        printf("report generated\n");
        if(pr==1){
            printf("\n\nprinting report: %s\n",output);
        };
    }

    free(output);
    return 0;
}
