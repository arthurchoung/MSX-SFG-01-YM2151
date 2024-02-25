#include <stdio.h>
#include <stdlib.h>

void generate_longwait(int nn0, int nn1)
{
    int len = nn0 + nn1*256;
    fprintf(stdout, "    ; longwait 0x%x 0x%x\n", nn0, nn1);
    for (;;) {
        if (len > 0x7ff) {
            fprintf(stdout, "    db 7\n");
            fprintf(stdout, "    db 255\n");
            len -= 0x7ff;
            continue;
        }
        fprintf(stdout, "    db %d\n", len/256);
        fprintf(stdout, "    db %d\n", len%256);
        break;
    }
}
void generate_wait(int len)
{
    fprintf(stdout, "    ; wait 0x%x\n", len);
    fprintf(stdout, "    db 0\n");
    fprintf(stdout, "    db %d\n", len);
}
void generate_write_register(int aa, int dd)
{
    if (aa == 0x14) { //IRQEN
        dd &= 0xf3;
    } else if (aa == 0x1b) { //CT
        dd &= 0x3f;
    }
    fprintf(stdout, "    ; aa %2x dd %2x\n", aa, dd);
    fprintf(stdout, "    db %d\n", aa);
    fprintf(stdout, "    db %d\n", dd);
}

void main()
{
    unsigned char header[256];
//    int result = fread(header, 1, 256, stdin);
    int result = fread(header, 1, 0x40, stdin);
    if (result != 0x40) {
        fprintf(stderr, "unable to read header\n");
        exit(1);
    }

    fprintf(stderr, "GD3 offset %2x %2x %2x %2x (%2x)\n",
        header[0x14+0],
        header[0x14+1],
        header[0x14+2],
        header[0x14+3],
        header[0x14+0]+header[0x14+1]*256+0x14);

    int dataoffset = header[0x34+0]+header[0x34+1]*256+0x34; // should use all 4 bytes
    fprintf(stderr, "VGM data offset %2x %2x %2x %2x (%x)\n",
        header[0x34+0],
        header[0x34+1],
        header[0x34+2],
        header[0x34+3],
        dataoffset);

    if (dataoffset-0x40 > 0) {
        result = fread(header+0x40, 1, dataoffset-0x40, stdin);
        if (result != dataoffset-0x40) {
            fprintf(stderr, "unable to read rest of header\n");
            exit(1);
        }
    }


    unsigned char cmd;
    int i = 256;
    unsigned char lowestaa = 255;
    unsigned char highestwait = 0;
    unsigned char lowestwait = 255;
    for(;;) {
        int j = 0;
        result = fread(&cmd, 1, 1, stdin);
        if (result != 1) {
            exit(0);
        }
        j++;
        if ((cmd >= 0x70) && (cmd <= 0x7f)) {
            int len = cmd-0x70+1;
            fprintf(stderr, "i %d wait %d\n", i, len);
            generate_wait(len);
        } else if (cmd == 0x54) {
            unsigned char aa;
            unsigned char dd;
            result = fread(&aa, 1, 1, stdin);
            if (result != 1) {
                fprintf(stderr, "unable to read data\n");
                exit(1);
            }
            result = fread(&dd, 1, 1, stdin);
            if (result != 1) {
                fprintf(stderr, "unable to read data\n");
                exit(1);
            }
            j+=2;
            fprintf(stderr, "i %d ym2151 aa %2x dd %2x\n", i, aa, dd);
            generate_write_register(aa, dd);
            if (aa < lowestaa) {
                lowestaa = aa;
            }
        } else if (cmd == 0x61) {
            unsigned char nn[2];
            result = fread(nn, 1, 2, stdin);
            if (result != 2) {
                fprintf(stderr, "unable to read data\n");
                exit(1);
            }
            j+=2;
            fprintf(stderr, "i %d longwait %2x %2x\n", i, nn[0], nn[1]);
            generate_longwait(nn[0], nn[1]);
            if (nn[1] == 0) {
                if (nn[0] < lowestwait) {
                    lowestwait = nn[0];
                }
            } else if (nn[1] > highestwait) {
                highestwait = nn[1];
            }
        } else if (cmd == 0x62) {
            fprintf(stderr, "i %d wait 735 samples\n", i);
            generate_longwait(0xdf, 0x02);
        } else if (cmd == 0x66) {
            fprintf(stderr, "i 0x%x end\n", i);
            fprintf(stdout, "    ; end\n");
            fprintf(stdout, "    db 0\n");
            fprintf(stdout, "    db 0\n");
            fprintf(stderr, "lowestaa 0x%x\n", lowestaa);
            fprintf(stderr, "lowestwait 0x%x\n", lowestwait);
            fprintf(stderr, "highestwait 0x%x\n", highestwait);
            exit(0);
        } else {
            fprintf(stderr, "i 0x%x unknown %2x\n", i, cmd);
            exit(1);
        }
        i+=j;
        
    }
    exit(0);
}

