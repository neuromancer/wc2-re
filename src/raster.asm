; @file
; @ingroup graphics
; @brief Hand-written raster library: RLE sprites, clipped lines, fills
; and ellipses, and IFF, PCX and GIF image decoding.
;
; Assembled rather than compiled: its shifts by one use the D1/D0
; encodings and it keeps tables in its code segment. It is linked
; between taperec.obj and intro.obj: code 40243c-407e31, data
; 490278-491c67. The SDL build uses the C counterparts in
; src/sdl/raster.c.
;
; Extracted from the retail executable by bin/extractRasterLibrary.py,
; which also checks that UASM reproduces its bytes and relocations.
; Regenerate rather than edit: the script holds the names and comments.

        .386
        .model  flat, c
        option  casemap:none

        public  CopyRasterDriverName
        public  InstallRasterDriverCallbacks
        public  SetRasterClipPixel
        public  ReadRasterClipPixel
        public  DrawClippedLine
        public  FillRasterClipCheckerboard
        public  DrawRLEImage
        public  DrawRLEImageUnclipped
        public  SetPaletteTranslationTable
        public  DrawRLEImageColor
        public  DrawRLEImageColorUnclipped
        public  RotateRLEImage
        public  GetRLEFrameBounds
        public  EncodeRasterClipToRLEFrame
        public  TranslateRLEFramePalette
        public  EncodeRLEScanline
        public  EmitRLEScanlineRun
        public  FillRasterClip
        public  BlitRasterClip
        public  ScrollRasterClipWrapped
        public  DrawRasterEllipse
        public  FillRasterEllipse
        public  GetRLETransformTrig
        public  CalculateRoundedRLEFixedProduct
        public  TransformRLEPoint
        public  GetRawImageHeight
        public  GetRawFrameWidth
        public  BlitRawFrame
        public  BlitSelectedRawFrames
        public  BlitRawScanline
        public  FindIFFChunkData
        public  DecodeIFFImage
        public  CopyILBMPalette
        public  GetILBMImageSize
        public  DecodePCXImage
        public  CopyPCXPaletteFromFileTail
        public  GetPCXImageSize
        public  ResetGIFLZWDictionary
        public  ReadGIFDataSubBlockByte
        public  ReadGIFLZWCode
        public  AppendGIFLZWDictionaryEntry
        public  EmitGIFDecodedPixel
        public  ExpandGIFLZWImage
        public  CopyGIFPalette
        public  GetGIFImageSize
        public  GetRLEFrameDimensions
        public  GetRLEFrameExtents
        public  GetRLEImageSize
        public  GetRLEImageOrigin
        public  ApplyRLEFramePalette
        public  CopyRLEFramePalette
        public  SetRLEFramePalette
        public  GetRLEFrameCount
        public  CollectUniqueRLEImageFrames
        public  CollectUniqueRLEPaletteFrames
        public  DissolveRasterClip
        public  FadeRasterPaletteToPalette
        public  CollectRasterClipColours
        public  g_anRLEQuarterCosine_00405cbb
        public  g_abIffBmhdChunkId_00406f74
        public  g_abIffCmapChunkId_00406f78
        public  g_abIffBodyChunkId_00406f7c
        public  g_apRasterDriverCallbacks_00490278
        public  g_szRasterDriverName_004902ac
        public  g_pRLEEncodeBuffer_004902b9
        public  g_nRLEPendingSkip_004902bd
        public  g_pRLEScanlineStart_004902c1
        public  g_pRLEScanCursor_004902c5
        public  g_pRLEOutputCursor_004902c9
        public  g_pRLELiteralStart_004902cd
        public  g_nRLEEncodedMinimumX_004902e1
        public  g_nRLEEncodedMinimumY_004902e5
        public  g_nRLEEncodedMaximumX_004902e9
        public  g_nRLEEncodedMaximumY_004902ed
        public  g_abRasterDecodeBuffer_004907f1
        public  g_abRasterPaletteUsageScratch_004914f1
        public  g_abRasterFadeAccumulator_004917f1
        public  g_abGifLZWCodeMasks_00491af1
        public  g_abGifInterlaceRowSteps_00491afa
        public  g_abGifInterlaceRowStarts_00491aff
        public  g_pGifRasterTarget_00491b04
        public  g_abRasterPaletteTranslation_00491b08
        public  g_aRLETransformVertices_00491c08
        public  g_anRLESourceSteps_00491c58

        .code

; Function start: 0x0040243C
; char *CopyRasterDriverName(void *const *callbacks);
; Unreferenced. Copies the raster driver name out of its callback table.
CopyRasterDriverName proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        call dword ptr [esi]
        mov edi, offset g_szRasterDriverName_004902ac
L402450:
        mov bl, byte ptr [eax]
        mov byte ptr [edi], bl
        inc edi
        inc eax
        or bl, bl
        jne L402450
        mov eax, offset g_szRasterDriverName_004902ac
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CopyRasterDriverName endp

; Function start: 0x00402465
; void InstallRasterDriverCallbacks(void *const *callbacks);
; Unreferenced. Copies thirteen driver callbacks into the library.
InstallRasterDriverCallbacks proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov edi, offset g_apRasterDriverCallbacks_00490278
        mov ecx, 0Dh
        rep movsd
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
InstallRasterDriverCallbacks endp

; Function start: 0x00402484
; unsigned int SetRasterClipPixel(RasterClip *clip, int x, int y, int colour);
; Writes one pixel through a clip, with explicit segment setup.
SetRasterClipPixel proc
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-18h], eax
        jle L402503
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L402503
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-1Ch], eax
        cmp eax, 0
        jg L4024B7
        mov eax, 0
L4024B7:
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-20h], eax
        cmp eax, 0
        jg L4024CA
        mov eax, 0
L4024CA:
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-18h]
        dec edx
        cmp eax, edx
        jl L4024DA
        mov eax, edx
L4024DA:
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L4024E9
        mov eax, edx
L4024E9:
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [ebp-0Ch]
        cmp eax, dword ptr [ebp-4]
        jl L40250E
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [ebp-8]
        jl L40250E
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-14h], eax
        jmp L402519
L402503:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L40250E:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L402519:
        mov ecx, dword ptr [ebp+0Ch]
        mov ebx, dword ptr [ebp+10h]
        add ecx, dword ptr [ebp-1Ch]
        add ebx, dword ptr [ebp-20h]
        cmp ecx, dword ptr [ebp-4]
        jl L402554
        cmp ecx, dword ptr [ebp-0Ch]
        jg L402554
        cmp ebx, dword ptr [ebp-8]
        jl L402554
        cmp ebx, dword ptr [ebp-10h]
        jg L402554
        mov eax, ebx
        imul dword ptr [ebp-18h]
        add eax, dword ptr [ebp-14h]
        add eax, ecx
        mov ebx, eax
        xor eax, eax
        mov al, byte ptr [ebx]
        mov dl, byte ptr [ebp+14h]
        mov byte ptr [ebx], dl
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L402554:
        mov eax, 0FFFFFFFDh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
SetRasterClipPixel endp

; Function start: 0x0040255F
; int ReadRasterClipPixel(RasterClip *clip, int x, int y);
; Reads one pixel through a clip.
ReadRasterClipPixel proc
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-18h], eax
        jle L4025DE
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L4025DE
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-1Ch], eax
        cmp eax, 0
        jg L402592
        mov eax, 0
L402592:
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-20h], eax
        cmp eax, 0
        jg L4025A5
        mov eax, 0
L4025A5:
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-18h]
        dec edx
        cmp eax, edx
        jl L4025B5
        mov eax, edx
L4025B5:
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L4025C4
        mov eax, edx
L4025C4:
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [ebp-0Ch]
        cmp eax, dword ptr [ebp-4]
        jl L4025E9
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [ebp-8]
        jl L4025E9
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-14h], eax
        jmp L4025F4
L4025DE:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4025E9:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4025F4:
        mov ecx, dword ptr [ebp+0Ch]
        mov ebx, dword ptr [ebp+10h]
        add ecx, dword ptr [ebp-1Ch]
        add ebx, dword ptr [ebp-20h]
        cmp ecx, dword ptr [ebp-4]
        jl L40262A
        cmp ecx, dword ptr [ebp-0Ch]
        jg L40262A
        cmp ebx, dword ptr [ebp-8]
        jl L40262A
        cmp ebx, dword ptr [ebp-10h]
        jg L40262A
        mov eax, ebx
        imul dword ptr [ebp-18h]
        add eax, dword ptr [ebp-14h]
        add eax, ecx
        mov ebx, eax
        xor eax, eax
        mov al, byte ptr [ebx]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L40262A:
        mov eax, 0FFFFFFFDh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
ReadRasterClipPixel endp

; Function start: 0x00402635
; unsigned int DrawClippedLine(RasterClip *clip, int x1, int y1, int x2, int
;                              y2, int mode, int colour);
; Draws a clipped line: mode 0 writes a colour, mode 1 translates through
; a colour table, and higher modes call the callback passed as the colour.
DrawClippedLine proc
        push ebp
        mov ebp, esp
        add esp, -58h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-50h], eax
        jle L4026B4
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L4026B4
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-54h], eax
        cmp eax, 0
        jg L402668
        mov eax, 0
L402668:
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-58h], eax
        cmp eax, 0
        jg L40267B
        mov eax, 0
L40267B:
        mov dword ptr [ebp-40h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-50h]
        dec edx
        cmp eax, edx
        jl L40268B
        mov eax, edx
L40268B:
        mov dword ptr [ebp-44h], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L40269A
        mov eax, edx
L40269A:
        mov dword ptr [ebp-48h], eax
        mov eax, dword ptr [ebp-44h]
        cmp eax, dword ptr [ebp-3Ch]
        jl L4026BF
        mov eax, dword ptr [ebp-48h]
        cmp eax, dword ptr [ebp-40h]
        jl L4026BF
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-4Ch], eax
        jmp L4026CA
L4026B4:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4026BF:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4026CA:
        mov eax, dword ptr [ebp-54h]
        add dword ptr [ebp+0Ch], eax
        add dword ptr [ebp+14h], eax
        mov eax, dword ptr [ebp-58h]
        add dword ptr [ebp+10h], eax
        add dword ptr [ebp+18h], eax
        mov eax, dword ptr [ebp+14h]
        sub eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-4], eax
        cdq
        mov dword ptr [ebp-0Ch], edx
        xor eax, edx
        sub eax, edx
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [ebp+18h]
        sub eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-10h], eax
        cdq
        mov dword ptr [ebp-18h], edx
        xor eax, edx
        sub eax, edx
        mov dword ptr [ebp-14h], eax
        mov eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-24h], eax
        mov eax, dword ptr [ebp+14h]
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-28h], eax
        mov eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-30h], eax
        cmp dword ptr [ebp-4], 0
        je L402E4B
        cmp dword ptr [ebp-10h], 0
        je L402ED6
        mov eax, dword ptr [ebp-0Ch]
        xor eax, dword ptr [ebp-18h]
        mov dword ptr [ebp-1Ch], eax
        mov edx, dword ptr [ebp-8]
        mov ebx, dword ptr [ebp-14h]
        mov eax, 0FFFFFFFFh
        cmp edx, ebx
        je L402750
        jl L40274C
        xchg edx, ebx
L40274C:
        xor eax, eax
        div ebx
L402750:
        mov dword ptr [ebp-20h], eax
        mov dword ptr [ebp-34h], 0
L40275A:
        xor edx, edx
        mov eax, dword ptr [ebp-24h]
        sub eax, dword ptr [ebp-3Ch]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-44h]
        sub eax, dword ptr [ebp-24h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [ebp-40h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-48h]
        sub eax, dword ptr [ebp-28h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-2Ch]
        sub eax, dword ptr [ebp-3Ch]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-44h]
        sub eax, dword ptr [ebp-2Ch]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-30h]
        sub eax, dword ptr [ebp-40h]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-48h]
        sub eax, dword ptr [ebp-30h]
        shl eax, 1
        adc dh, dh
        or dword ptr [ebp-34h], edx
        or edx, edx
        je L402B19
        test dl, dh
        jne L40302C
        mov ebx, dword ptr [ebp-8]
        cmp ebx, dword ptr [ebp-14h]
        jl L402814
        test dl, 8
        jne L40285D
        test dl, 4
        jne L4028B1
        test dl, 2
        jne L402935
        test dl, 1
        jne L40298D
        test dh, 8
        jne L4029BD
        test dh, 4
        jne L402A18
        test dh, 2
        jne L402A97
        test dh, 1
        jne L402AEE
        jmp L40275A
L402814:
        test dl, 8
        jne L402885
        test dl, 4
        jne L4028DD
        test dl, 2
        jne L40290D
        test dl, 1
        jne L402961
        test dh, 8
        jne L4029E9
        test dh, 4
        jne L402A40
        test dh, 2
        jne L402A6B
        test dh, 1
        jne L402AC6
        jmp L40275A
L40285D:
        mov eax, dword ptr [ebp-3Ch]
        mov dword ptr [ebp-24h], eax
        sub eax, dword ptr [ebp+0Ch]
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-28h], eax
        jmp L40275A
L402885:
        mov eax, dword ptr [ebp-3Ch]
        mov dword ptr [ebp-24h], eax
        sub eax, dword ptr [ebp+0Ch]
        mov edx, eax
        dec edx
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, -1
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-28h], eax
        jmp L40275A
L4028B1:
        mov eax, dword ptr [ebp-44h]
        mov dword ptr [ebp-24h], eax
        sub eax, dword ptr [ebp+0Ch]
        neg eax
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-28h], eax
        jmp L40275A
L4028DD:
        mov eax, dword ptr [ebp-44h]
        mov dword ptr [ebp-24h], eax
        sub eax, dword ptr [ebp+0Ch]
        neg eax
        mov edx, eax
        dec edx
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, -1
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-28h], eax
        jmp L40275A
L40290D:
        mov eax, dword ptr [ebp-40h]
        mov dword ptr [ebp-28h], eax
        sub eax, dword ptr [ebp+10h]
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-24h], eax
        jmp L40275A
L402935:
        mov eax, dword ptr [ebp-40h]
        mov dword ptr [ebp-28h], eax
        sub eax, dword ptr [ebp+10h]
        mov edx, eax
        dec edx
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, -1
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-24h], eax
        jmp L40275A
L402961:
        mov eax, dword ptr [ebp-48h]
        mov dword ptr [ebp-28h], eax
        sub eax, dword ptr [ebp+10h]
        neg eax
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-24h], eax
        jmp L40275A
L40298D:
        mov eax, dword ptr [ebp-48h]
        mov dword ptr [ebp-28h], eax
        sub eax, dword ptr [ebp+10h]
        neg eax
        mov edx, eax
        dec edx
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, -1
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-24h], eax
        jmp L40275A
L4029BD:
        mov eax, dword ptr [ebp-3Ch]
        mov dword ptr [ebp-2Ch], eax
        sub eax, dword ptr [ebp+0Ch]
        neg eax
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-30h], eax
        jmp L40275A
L4029E9:
        mov eax, dword ptr [ebp-3Ch]
        mov dword ptr [ebp-2Ch], eax
        sub eax, dword ptr [ebp+0Ch]
        neg eax
        mov edx, eax
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, 0
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-30h], eax
        jmp L40275A
L402A18:
        mov eax, dword ptr [ebp-44h]
        mov dword ptr [ebp-2Ch], eax
        sub eax, dword ptr [ebp+0Ch]
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-30h], eax
        jmp L40275A
L402A40:
        mov eax, dword ptr [ebp-44h]
        mov dword ptr [ebp-2Ch], eax
        sub eax, dword ptr [ebp+0Ch]
        mov edx, eax
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, 0
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-30h], eax
        jmp L40275A
L402A6B:
        mov eax, dword ptr [ebp-40h]
        mov dword ptr [ebp-30h], eax
        sub eax, dword ptr [ebp+10h]
        neg eax
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-2Ch], eax
        jmp L40275A
L402A97:
        mov eax, dword ptr [ebp-40h]
        mov dword ptr [ebp-30h], eax
        sub eax, dword ptr [ebp+10h]
        neg eax
        mov edx, eax
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, 0
        mov edx, dword ptr [ebp-1Ch]
        not edx
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-2Ch], eax
        jmp L40275A
L402AC6:
        mov eax, dword ptr [ebp-48h]
        mov dword ptr [ebp-30h], eax
        sub eax, dword ptr [ebp+10h]
        mul dword ptr [ebp-20h]
        add eax, 80000000h
        adc edx, 0
        mov eax, edx
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-2Ch], eax
        jmp L40275A
L402AEE:
        mov eax, dword ptr [ebp-48h]
        mov dword ptr [ebp-30h], eax
        sub eax, dword ptr [ebp+10h]
        mov edx, eax
        mov eax, 80000000h
        div dword ptr [ebp-20h]
        cmp edx, 1
        sbb eax, 0
        mov edx, dword ptr [ebp-1Ch]
        xor eax, edx
        sub eax, edx
        add eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-2Ch], eax
        jmp L40275A
L402B19:
        mov eax, dword ptr [ebp-28h]
        imul dword ptr [ebp-50h]
        add eax, dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-24h]
        mov edi, eax
        mov esi, dword ptr [ebp-50h]
        xor esi, dword ptr [ebp-18h]
        sub esi, dword ptr [ebp-18h]
        mov ebx, dword ptr [ebp-20h]
        mov eax, dword ptr [ebp-8]
        cmp eax, dword ptr [ebp-14h]
        je L402F5C
        jg L402CAD
        mov eax, dword ptr [ebp-30h]
        sub eax, dword ptr [ebp-28h]
        cdq
        xor eax, edx
        sub eax, edx
        inc eax
        mov ecx, eax
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [ebp+10h]
        cdq
        xor eax, edx
        sub eax, edx
        mul ebx
        add eax, 80000000h
        mov edx, eax
        cmp dword ptr [ebp-0Ch], -1
        je L402C0E
        cmp dword ptr [ebp+1Ch], 1
        je L402BA5
        jg L402BE3
        mov eax, dword ptr [ebp+20h]
L402B7C:
        mov byte ptr [edi], al
        add edx, ebx
        adc edi, esi
        dec ecx
        je L402BA0
        mov byte ptr [edi], al
        add edx, ebx
        adc edi, esi
        dec ecx
        je L402BA0
        mov byte ptr [edi], al
        add edx, ebx
        adc edi, esi
        dec ecx
        je L402BA0
        mov byte ptr [edi], al
        add edx, ebx
        adc edi, esi
        dec ecx
        jne L402B7C
L402BA0:
        jmp L40301D
L402BA5:
        mov eax, dword ptr [ebp+20h]
        push ebp
        mov ebp, ebx
        mov ebx, eax
L402BAD:
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        adc edi, esi
        dec ecx
        je L402BDD
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        adc edi, esi
        dec ecx
        je L402BDD
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        adc edi, esi
        dec ecx
        je L402BDD
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        adc edi, esi
        dec ecx
        jne L402BAD
L402BDD:
        pop ebp
        jmp L40301D
L402BE3:
        mov esi, dword ptr [ebp+8]
        mov edi, dword ptr [ebp-24h]
        sub edi, dword ptr [esi+4]
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [esi+8]
        mov esi, eax
        mov eax, dword ptr [ebp-18h]
        add eax, eax
        inc eax
L402BFA:
        pushad
        call dword ptr [ebp+20h]
        popad
        add edx, ebx
        jae L402C04
        inc edi
L402C04:
        add esi, eax
        dec ecx
        jne L402BFA
        jmp L40301D
L402C0E:
        neg esi
        cmp dword ptr [ebp+1Ch], 1
        je L402C44
        jg L402C82
        mov eax, dword ptr [ebp+20h]
L402C1B:
        mov byte ptr [edi], al
        add edx, ebx
        sbb edi, esi
        dec ecx
        je L402C3F
        mov byte ptr [edi], al
        add edx, ebx
        sbb edi, esi
        dec ecx
        je L402C3F
        mov byte ptr [edi], al
        add edx, ebx
        sbb edi, esi
        dec ecx
        je L402C3F
        mov byte ptr [edi], al
        add edx, ebx
        sbb edi, esi
        dec ecx
        jne L402C1B
L402C3F:
        jmp L40301D
L402C44:
        mov eax, dword ptr [ebp+20h]
        push ebp
        mov ebp, ebx
        mov ebx, eax
L402C4C:
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        sbb edi, esi
        dec ecx
        je L402C7C
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        sbb edi, esi
        dec ecx
        je L402C7C
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        sbb edi, esi
        dec ecx
        je L402C7C
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edx, ebp
        sbb edi, esi
        dec ecx
        jne L402C4C
L402C7C:
        pop ebp
        jmp L40301D
L402C82:
        mov esi, dword ptr [ebp+8]
        mov edi, dword ptr [ebp-24h]
        sub edi, dword ptr [esi+4]
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [esi+8]
        mov esi, eax
        mov eax, dword ptr [ebp-18h]
        add eax, eax
        inc eax
L402C99:
        pushad
        call dword ptr [ebp+20h]
        popad
        add edx, ebx
        jae L402CA3
        dec edi
L402CA3:
        add esi, eax
        dec ecx
        jne L402C99
        jmp L40301D
L402CAD:
        mov eax, dword ptr [ebp-2Ch]
        sub eax, dword ptr [ebp-24h]
        cdq
        xor eax, edx
        sub eax, edx
        inc eax
        mov ecx, eax
        mov eax, dword ptr [ebp-24h]
        sub eax, dword ptr [ebp+0Ch]
        cdq
        xor eax, edx
        sub eax, edx
        mul ebx
        add eax, 80000000h
        mov edx, eax
        cmp dword ptr [ebp-0Ch], -1
        je L402D92
        cmp dword ptr [ebp+1Ch], 1
        je L402D1D
        jg L402D67
        mov eax, dword ptr [ebp+20h]
L402CE8:
        mov byte ptr [edi], al
        inc edi
        add edx, ebx
        jae L402CF1
        add edi, esi
L402CF1:
        dec ecx
        je L402D18
        mov byte ptr [edi], al
        inc edi
        add edx, ebx
        jae L402CFD
        add edi, esi
L402CFD:
        dec ecx
        je L402D18
        mov byte ptr [edi], al
        inc edi
        add edx, ebx
        jae L402D09
        add edi, esi
L402D09:
        dec ecx
        je L402D18
        mov byte ptr [edi], al
        inc edi
        add edx, ebx
        jae L402D15
        add edi, esi
L402D15:
        dec ecx
        jne L402CE8
L402D18:
        jmp L40301D
L402D1D:
        mov eax, dword ptr [ebp+20h]
        push ebp
        mov ebp, ebx
        mov ebx, eax
L402D25:
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        inc edi
        add edx, ebp
        jae L402D31
        add edi, esi
L402D31:
        dec ecx
        je L402D61
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        inc edi
        add edx, ebp
        jae L402D40
        add edi, esi
L402D40:
        dec ecx
        je L402D61
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        inc edi
        add edx, ebp
        jae L402D4F
        add edi, esi
L402D4F:
        dec ecx
        je L402D61
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        inc edi
        add edx, ebp
        jae L402D5E
        add edi, esi
L402D5E:
        dec ecx
        jne L402D25
L402D61:
        pop ebp
        jmp L40301D
L402D67:
        mov esi, dword ptr [ebp+8]
        mov edi, dword ptr [ebp-24h]
        sub edi, dword ptr [esi+4]
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [esi+8]
        mov esi, eax
        mov eax, dword ptr [ebp-18h]
        add eax, eax
        inc eax
L402D7E:
        pushad
        call dword ptr [ebp+20h]
        popad
        add edx, ebx
        jae L402D88
        inc esi
L402D88:
        add edi, eax
        dec ecx
        jne L402D7E
        jmp L40301D
L402D92:
        cmp dword ptr [ebp+1Ch], 1
        je L402DD6
        jg L402E20
        mov eax, dword ptr [ebp+20h]
L402DA1:
        mov byte ptr [edi], al
        dec edi
        add edx, ebx
        jae L402DAA
        add edi, esi
L402DAA:
        dec ecx
        je L402DD1
        mov byte ptr [edi], al
        dec edi
        add edx, ebx
        jae L402DB6
        add edi, esi
L402DB6:
        dec ecx
        je L402DD1
        mov byte ptr [edi], al
        dec edi
        add edx, ebx
        jae L402DC2
        add edi, esi
L402DC2:
        dec ecx
        je L402DD1
        mov byte ptr [edi], al
        dec edi
        add edx, ebx
        jae L402DCE
        add edi, esi
L402DCE:
        dec ecx
        jne L402DA1
L402DD1:
        jmp L40301D
L402DD6:
        mov eax, dword ptr [ebp+20h]
        push ebp
        mov ebp, ebx
        mov ebx, eax
L402DDE:
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        dec edi
        add edx, ebp
        jae L402DEA
        add edi, esi
L402DEA:
        dec ecx
        je L402E1A
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        dec edi
        add edx, ebp
        jae L402DF9
        add edi, esi
L402DF9:
        dec ecx
        je L402E1A
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        dec edi
        add edx, ebp
        jae L402E08
        add edi, esi
L402E08:
        dec ecx
        je L402E1A
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        dec edi
        add edx, ebp
        jae L402E17
        add edi, esi
L402E17:
        dec ecx
        jne L402DDE
L402E1A:
        pop ebp
        jmp L40301D
L402E20:
        mov esi, dword ptr [ebp+8]
        mov edi, dword ptr [ebp-24h]
        sub edi, dword ptr [esi+4]
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [esi+8]
        mov esi, eax
        mov eax, dword ptr [ebp-18h]
        add eax, eax
        inc eax
L402E37:
        pushad
        call dword ptr [ebp+20h]
        popad
        add edx, ebx
        jae L402E41
        dec esi
L402E41:
        add edi, eax
        dec ecx
        jne L402E37
        jmp L40301D
L402E4B:
        mov eax, dword ptr [ebp+0Ch]
        cmp eax, dword ptr [ebp-3Ch]
        jl L40302C
        cmp eax, dword ptr [ebp-44h]
        jg L40302C
        mov eax, dword ptr [ebp+10h]
        cmp eax, dword ptr [ebp+18h]
        jg L402E6B
        mov eax, dword ptr [ebp+18h]
L402E6B:
        cmp eax, dword ptr [ebp-40h]
        jl L40302C
        mov eax, dword ptr [ebp+10h]
        cmp eax, dword ptr [ebp+18h]
        jl L402E7F
        mov eax, dword ptr [ebp+18h]
L402E7F:
        cmp eax, dword ptr [ebp-48h]
        jg L40302C
        mov eax, dword ptr [ebp+10h]
        cmp eax, dword ptr [ebp-40h]
        jg L402E93
        mov eax, dword ptr [ebp-40h]
L402E93:
        cmp eax, dword ptr [ebp-48h]
        jl L402E9B
        mov eax, dword ptr [ebp-48h]
L402E9B:
        mov dword ptr [ebp-28h], eax
        mov eax, dword ptr [ebp+18h]
        cmp eax, dword ptr [ebp-40h]
        jg L402EA9
        mov eax, dword ptr [ebp-40h]
L402EA9:
        cmp eax, dword ptr [ebp-48h]
        jl L402EB1
        mov eax, dword ptr [ebp-48h]
L402EB1:
        mov dword ptr [ebp-30h], eax
        mov eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-24h], eax
        mov esi, dword ptr [ebp-50h]
        xor esi, dword ptr [ebp-18h]
        sub esi, dword ptr [ebp-18h]
        mov eax, dword ptr [ebp-30h]
        sub eax, dword ptr [ebp-28h]
        cdq
        xor eax, edx
        sub eax, edx
        mov ecx, eax
        inc ecx
        jmp L402F7C
L402ED6:
        mov eax, dword ptr [ebp+10h]
        cmp eax, dword ptr [ebp-40h]
        jl L40302C
        cmp eax, dword ptr [ebp-48h]
        jg L40302C
        mov eax, dword ptr [ebp+0Ch]
        cmp eax, dword ptr [ebp+14h]
        jg L402EF6
        mov eax, dword ptr [ebp+14h]
L402EF6:
        cmp eax, dword ptr [ebp-3Ch]
        jl L40302C
        mov eax, dword ptr [ebp+0Ch]
        cmp eax, dword ptr [ebp+14h]
        jl L402F0A
        mov eax, dword ptr [ebp+14h]
L402F0A:
        cmp eax, dword ptr [ebp-44h]
        jg L40302C
        mov eax, dword ptr [ebp+0Ch]
        cmp eax, dword ptr [ebp-3Ch]
        jg L402F1E
        mov eax, dword ptr [ebp-3Ch]
L402F1E:
        cmp eax, dword ptr [ebp-44h]
        jl L402F26
        mov eax, dword ptr [ebp-44h]
L402F26:
        mov dword ptr [ebp-24h], eax
        mov eax, dword ptr [ebp+14h]
        cmp eax, dword ptr [ebp-3Ch]
        jg L402F34
        mov eax, dword ptr [ebp-3Ch]
L402F34:
        cmp eax, dword ptr [ebp-44h]
        jl L402F3C
        mov eax, dword ptr [ebp-44h]
L402F3C:
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-28h], eax
        mov esi, dword ptr [ebp-0Ch]
        inc esi
        or esi, dword ptr [ebp-0Ch]
        mov eax, dword ptr [ebp-2Ch]
        sub eax, dword ptr [ebp-24h]
        cdq
        xor eax, edx
        sub eax, edx
        mov ecx, eax
        inc ecx
        jmp L402F7C
L402F5C:
        mov esi, dword ptr [ebp-50h]
        xor esi, dword ptr [ebp-18h]
        sub esi, dword ptr [ebp-18h]
        mov eax, dword ptr [ebp-0Ch]
        inc eax
        or eax, dword ptr [ebp-0Ch]
        add esi, eax
        mov eax, dword ptr [ebp-2Ch]
        sub eax, dword ptr [ebp-24h]
        cdq
        xor eax, edx
        sub eax, edx
        mov ecx, eax
        inc ecx
L402F7C:
        mov eax, dword ptr [ebp-28h]
        imul dword ptr [ebp-50h]
        add eax, dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-24h]
        mov edi, eax
        cmp dword ptr [ebp+1Ch], 1
        je L402FB3
        jg L402FE0
        mov eax, dword ptr [ebp+20h]
L402F95:
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        je L402FB1
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        je L402FB1
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        je L402FB1
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        jne L402F95
L402FB1:
        jmp L40301D
L402FB3:
        mov ebx, dword ptr [ebp+20h]
L402FB6:
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        je L402FDE
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        je L402FDE
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        je L402FDE
        mov al, byte ptr [edi]
        xlatb
        mov byte ptr [edi], al
        add edi, esi
        dec ecx
        jne L402FB6
L402FDE:
        jmp L40301D
L402FE0:
        mov esi, dword ptr [ebp+8]
        mov edi, dword ptr [ebp-24h]
        sub edi, dword ptr [esi+4]
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [esi+8]
        mov esi, eax
        xor eax, eax
        test dword ptr [ebp-10h], 0FFFFFFFFh
        setne al
        or eax, dword ptr [ebp-18h]
        xor ebx, ebx
        test dword ptr [ebp-4], 0FFFFFFFFh
        setne bl
        or ebx, dword ptr [ebp-0Ch]
L40300F:
        pushad
        call dword ptr [ebp+20h]
        popad
        add esi, eax
        add edi, ebx
        dec ecx
        jne L40300F
        jmp L40301D
L40301D:
        xor eax, eax
        cmp dword ptr [ebp-34h], 1
        setae al
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L40302C:
        mov eax, 2
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DrawClippedLine endp

; Function start: 0x00403037
; int FillRasterClipCheckerboard(RasterClip *clip, int left, int top, int
;                                right, int bottom, unsigned char colour);
; Fills a clip with a two-colour checkerboard.
FillRasterClipCheckerboard proc
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-18h], eax
        jle L4030B6
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L4030B6
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-1Ch], eax
        cmp eax, 0
        jg L40306A
        mov eax, 0
L40306A:
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-20h], eax
        cmp eax, 0
        jg L40307D
        mov eax, 0
L40307D:
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-18h]
        dec edx
        cmp eax, edx
        jl L40308D
        mov eax, edx
L40308D:
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L40309C
        mov eax, edx
L40309C:
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [ebp-0Ch]
        cmp eax, dword ptr [ebp-4]
        jl L4030C1
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [ebp-8]
        jl L4030C1
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-14h], eax
        jmp L4030CC
L4030B6:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4030C1:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4030CC:
        mov eax, dword ptr [ebp-1Ch]
        add dword ptr [ebp+0Ch], eax
        add dword ptr [ebp+14h], eax
        mov eax, dword ptr [ebp-20h]
        add dword ptr [ebp+10h], eax
        add dword ptr [ebp+18h], eax
        mov eax, dword ptr [ebp-4]
        cmp dword ptr [ebp+0Ch], eax
        jg L4030E9
        mov dword ptr [ebp+0Ch], eax
L4030E9:
        mov eax, dword ptr [ebp-8]
        cmp dword ptr [ebp+10h], eax
        jg L4030F4
        mov dword ptr [ebp+10h], eax
L4030F4:
        mov eax, dword ptr [ebp-0Ch]
        cmp dword ptr [ebp+14h], eax
        jl L4030FF
        mov dword ptr [ebp+14h], eax
L4030FF:
        mov eax, dword ptr [ebp-10h]
        cmp dword ptr [ebp+18h], eax
        jl L40310A
        mov dword ptr [ebp+18h], eax
L40310A:
        mov ecx, dword ptr [ebp+14h]
        sub ecx, dword ptr [ebp+0Ch]
        jl L403165
        inc ecx
        mov eax, dword ptr [ebp+10h]
        imul dword ptr [ebp-18h]
        add eax, dword ptr [ebp-14h]
        add eax, dword ptr [ebp+0Ch]
        mov edi, eax
        mov edx, dword ptr [ebp+18h]
        sub edx, dword ptr [ebp+10h]
        jl L403165
        mov eax, dword ptr [ebp+1Ch]
        mov esi, edi
        mov ebx, ecx
        jmp L403139
L403132:
        add esi, dword ptr [ebp-18h]
        mov edi, esi
        mov ecx, ebx
L403139:
        push edx
        and edx, 1
        je L403144
        pop edx
        inc edi
        dec ecx
        jmp L403145
L403144:
        pop edx
L403145:
        mov byte ptr [edi], al
        add edi, 2
        sub ecx, 2
        jg L403145
        dec edx
        jns L403132
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
        mov eax, 0FFFFFFFDh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L403165:
        mov eax, 0FFFFFFFCh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
FillRasterClipCheckerboard endp

; Function start: 0x00403170
; int DrawRLEImage(RasterClip *clip, unsigned char *shape, int frame, int x,
;                  int y);
; Draws a clipped RLE frame.
DrawRLEImage proc
        push ebp
        mov ebp, esp
        add esp, -50h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-48h], eax
        jle L4031EF
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L4031EF
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-4Ch], eax
        cmp eax, 0
        jg L4031A3
        mov eax, 0
L4031A3:
        mov dword ptr [ebp-34h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-50h], eax
        cmp eax, 0
        jg L4031B6
        mov eax, 0
L4031B6:
        mov dword ptr [ebp-38h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-48h]
        dec edx
        cmp eax, edx
        jl L4031C6
        mov eax, edx
L4031C6:
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L4031D5
        mov eax, edx
L4031D5:
        mov dword ptr [ebp-40h], eax
        mov eax, dword ptr [ebp-3Ch]
        cmp eax, dword ptr [ebp-34h]
        jl L4031FA
        mov eax, dword ptr [ebp-40h]
        cmp eax, dword ptr [ebp-38h]
        jl L4031FA
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-44h], eax
        jmp L403205
L4031EF:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4031FA:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L403205:
        mov eax, dword ptr [ebp-4Ch]
        add dword ptr [ebp+14h], eax
        mov eax, dword ptr [ebp-50h]
        add dword ptr [ebp+18h], eax
        mov esi, dword ptr [ebp+10h]
        shl esi, 3
        add esi, 8
        add esi, dword ptr [ebp+0Ch]
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-30h], esi
        mov eax, dword ptr [esi+8]
        add eax, dword ptr [ebp+14h]
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+0Ch]
        add eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [esi+10h]
        add eax, dword ptr [ebp+14h]
        mov dword ptr [ebp-14h], eax
        mov eax, dword ptr [esi+14h]
        add eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-18h], eax
        add esi, 18h
        mov eax, dword ptr [ebp-14h]
        cmp eax, dword ptr [ebp-0Ch]
        jl L403569
        mov eax, dword ptr [ebp-18h]
        cmp eax, dword ptr [ebp-10h]
        jl L403569
        xor edx, edx
        mov eax, dword ptr [ebp-0Ch]
        sub eax, dword ptr [ebp-34h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-3Ch]
        sub eax, dword ptr [ebp-0Ch]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-10h]
        sub eax, dword ptr [ebp-38h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-40h]
        sub eax, dword ptr [ebp-10h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-14h]
        sub eax, dword ptr [ebp-34h]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-3Ch]
        sub eax, dword ptr [ebp-14h]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-18h]
        sub eax, dword ptr [ebp-38h]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-40h]
        sub eax, dword ptr [ebp-18h]
        shl eax, 1
        adc dh, dh
        mov dword ptr [ebp-1Ch], edx
        test dl, dh
        jne L40355E
        or dl, dh
        jne L4032F0
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+4]
        sub dword ptr [ebp+14h], eax
        mov eax, dword ptr [esi+8]
        sub dword ptr [ebp+18h], eax
        push dword ptr [ebp-48h]
        push dword ptr [ebp+18h]
        push dword ptr [ebp+14h]
        push dword ptr [ebp-30h]
        push dword ptr [ebp+8]
        call DrawRLEImageUnclipped
        add esp, 14h
        jmp L403556
L4032F0:
        mov eax, dword ptr [ebp-10h]
        imul dword ptr [ebp-48h]
        add eax, dword ptr [ebp-44h]
        add eax, dword ptr [ebp-0Ch]
        mov edi, eax
        mov ecx, dword ptr [ebp-10h]
        mov dword ptr [ebp-20h], ecx
        jmp L40331C
L403306:
        movzx eax, al
        add esi, eax
        dec esi
L40330C:
        inc esi
L40330D:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40330C
        jne L403306
        jb L40330C
        add edi, dword ptr [ebp-48h]
        inc ecx
L40331C:
        cmp ecx, dword ptr [ebp-38h]
        jl L40330D
        mov dword ptr [ebp-24h], edi
        mov dword ptr [ebp-20h], ecx
        mov eax, edi
        sub eax, dword ptr [ebp-0Ch]
        add eax, dword ptr [ebp-34h]
        mov dword ptr [ebp-28h], eax
        mov eax, edi
        sub eax, dword ptr [ebp-0Ch]
        add eax, dword ptr [ebp-3Ch]
        mov dword ptr [ebp-2Ch], eax
        jmp L40354A
L403342:
        mov eax, dword ptr [ebp-20h]
        cmp eax, dword ptr [ebp-40h]
        jg L403556
        mov edi, dword ptr [ebp-24h]
        test dword ptr [ebp-1Ch], 8
        jne L4033CF
        test dword ptr [ebp-1Ch], 400h
        jne L403459
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403387
        jne L4033B0
        jae L4033CA
L403372:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403387
        jne L4033B0
        jb L403372
        jae L4033CA
L403387:
        movzx ecx, al
L40338A:
        mov al, byte ptr [esi]
        inc esi
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403387
        jae L4033CA
        je L403372
L4033B0:
        movzx ecx, al
L4033B3:
        push ecx
        and ecx, 3
        rep movsb
        pop ecx
        shr ecx, 2
        rep movsd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403387
        jne L4033B0
        jb L403372
L4033CA:
        jmp L40353B
L4033CF:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4033EF
        jne L403420
        jae L403454
L4033DA:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4033EF
        jne L403420
        jb L4033DA
        jae L403454
L4033EF:
        movzx ecx, al
        mov eax, dword ptr [ebp-28h]
        sub eax, edi
        cmp eax, ecx
        jge L403412
        or eax, eax
        js L403403
        add edi, eax
        sub ecx, eax
L403403:
        test dword ptr [ebp-1Ch], 400h
        je L40338A
        jne L403480
L403412:
        add edi, ecx
        inc esi
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4033EF
        jae L403454
        je L4033DA
L403420:
        movzx ecx, al
        mov eax, dword ptr [ebp-28h]
        sub eax, edi
        cmp eax, ecx
        jge L403445
        or eax, eax
        js L403436
        add edi, eax
        sub ecx, eax
        add esi, eax
L403436:
        test dword ptr [ebp-1Ch], 400h
        je L4033B3
        jne L4034C3
L403445:
        add edi, ecx
        add esi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4033EF
        jne L403420
        jb L4033DA
L403454:
        jmp L40353B
L403459:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40347D
        jne L4034C0
        jae L4034F6
L403468:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40347D
        jne L4034C0
        jb L403468
        jae L4034F6
L40347D:
        movzx ecx, al
L403480:
        cmp edi, dword ptr [ebp-2Ch]
        jg L40351B
        mov eax, edi
        add eax, ecx
        dec eax
        sub eax, dword ptr [ebp-2Ch]
        cdq
        not edx
        and edx, eax
        sub ecx, edx
        mov al, byte ptr [esi]
        inc esi
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        add edi, edx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40347D
        jae L4034F6
        je L403468
L4034C0:
        movzx ecx, al
L4034C3:
        cmp edi, dword ptr [ebp-2Ch]
        jg L40352C
        mov eax, edi
        add eax, ecx
        dec eax
        sub eax, dword ptr [ebp-2Ch]
        cdq
        not edx
        and edx, eax
        sub ecx, edx
        push ecx
        and ecx, 3
        rep movsb
        pop ecx
        shr ecx, 2
        rep movsd
        add edi, edx
        add esi, edx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40347D
        jne L4034C0
        jb L403468
L4034F6:
        jmp L40353B
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403518
        jne L403529
        jae L40353B
L403503:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403518
        jne L403529
        jb L403503
        jae L40353B
L403518:
        movzx ecx, al
L40351B:
        add edi, ecx
        inc esi
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403518
        jae L40353B
        je L403503
L403529:
        movzx ecx, al
L40352C:
        add edi, ecx
        add esi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403518
        jne L403529
        jb L403503
L40353B:
        mov eax, dword ptr [ebp-48h]
        add dword ptr [ebp-24h], eax
        add dword ptr [ebp-28h], eax
        add dword ptr [ebp-2Ch], eax
        inc dword ptr [ebp-20h]
L40354A:
        mov eax, dword ptr [ebp-20h]
        cmp eax, dword ptr [ebp-18h]
        jle L403342
L403556:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L40355E:
        mov eax, 0FFFFFFFDh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L403569:
        mov eax, 0FFFFFFFCh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DrawRLEImage endp

; Function start: 0x00403574
; int DrawRLEImageUnclipped(RasterClip *clip, RLEFrameHeader *frameHeader, int
;                           x, int y, int strideScratch);
; Draws an RLE frame that lies entirely inside the clip.
DrawRLEImageUnclipped proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+4]
        add dword ptr [ebp+10h], eax
        mov eax, dword ptr [esi+8]
        add dword ptr [ebp+14h], eax
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp+18h], eax
        jle L403633
        mov esi, dword ptr [ebp+0Ch]
        mov edi, dword ptr [ebx]
        mov eax, dword ptr [esi+8]
        add eax, dword ptr [ebp+10h]
        add edi, eax
        mov eax, dword ptr [esi+0Ch]
        mov ebx, eax
        add eax, dword ptr [ebp+14h]
        mul dword ptr [ebp+18h]
        add edi, eax
        mov edx, edi
        mov eax, dword ptr [esi+10h]
        mov eax, dword ptr [esi+14h]
        inc eax
        sub eax, ebx
        mov ebx, eax
        jle L403633
        add esi, 18h
L4035C8:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4035E8
        jne L403611
        jae L40362B
L4035D3:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4035E8
        jne L403611
        jb L4035D3
        jae L40362B
L4035E8:
        movzx ecx, al
        mov al, byte ptr [esi]
        inc esi
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4035E8
        jae L40362B
        je L4035D3
L403611:
        movzx ecx, al
        push ecx
        and ecx, 3
        rep movsb
        pop ecx
        shr ecx, 2
        rep movsd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4035E8
        jne L403611
        jb L4035D3
L40362B:
        add edx, dword ptr [ebp+18h]
        mov edi, edx
        dec ebx
        jne L4035C8
L403633:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DrawRLEImageUnclipped endp

; Function start: 0x0040363B
; void SetPaletteTranslationTable(const unsigned char *translation);
; Selects the colour translation table for the colour RLE blitters.
SetPaletteTranslationTable proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov edi, offset g_abRasterPaletteTranslation_00491b08
        mov ecx, 40h
        rep movsd
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
SetPaletteTranslationTable endp

; Function start: 0x0040365A
; int DrawRLEImageColor(RasterClip *clip, unsigned char *shape, int frame, int
;                       x, int y);
; Draws a clipped RLE frame through the colour translation table.
DrawRLEImageColor proc
        push ebp
        mov ebp, esp
        add esp, -50h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-48h], eax
        jle L4036D9
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L4036D9
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-4Ch], eax
        cmp eax, 0
        jg L40368D
        mov eax, 0
L40368D:
        mov dword ptr [ebp-34h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-50h], eax
        cmp eax, 0
        jg L4036A0
        mov eax, 0
L4036A0:
        mov dword ptr [ebp-38h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-48h]
        dec edx
        cmp eax, edx
        jl L4036B0
        mov eax, edx
L4036B0:
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L4036BF
        mov eax, edx
L4036BF:
        mov dword ptr [ebp-40h], eax
        mov eax, dword ptr [ebp-3Ch]
        cmp eax, dword ptr [ebp-34h]
        jl L4036E4
        mov eax, dword ptr [ebp-40h]
        cmp eax, dword ptr [ebp-38h]
        jl L4036E4
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-44h], eax
        jmp L4036EF
L4036D9:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4036E4:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L4036EF:
        mov eax, dword ptr [ebp-4Ch]
        add dword ptr [ebp+14h], eax
        mov eax, dword ptr [ebp-50h]
        add dword ptr [ebp+18h], eax
        mov esi, dword ptr [ebp+10h]
        shl esi, 3
        add esi, 8
        add esi, dword ptr [ebp+0Ch]
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-30h], esi
        mov eax, dword ptr [esi+8]
        add eax, dword ptr [ebp+14h]
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+0Ch]
        add eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [esi+10h]
        add eax, dword ptr [ebp+14h]
        mov dword ptr [ebp-14h], eax
        mov eax, dword ptr [esi+14h]
        add eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-18h], eax
        add esi, 18h
        mov eax, dword ptr [ebp-14h]
        cmp eax, dword ptr [ebp-0Ch]
        jl L403B27
        mov eax, dword ptr [ebp-18h]
        cmp eax, dword ptr [ebp-10h]
        jl L403B27
        xor edx, edx
        mov eax, dword ptr [ebp-0Ch]
        sub eax, dword ptr [ebp-34h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-3Ch]
        sub eax, dword ptr [ebp-0Ch]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-10h]
        sub eax, dword ptr [ebp-38h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-40h]
        sub eax, dword ptr [ebp-10h]
        shl eax, 1
        adc dl, dl
        mov eax, dword ptr [ebp-14h]
        sub eax, dword ptr [ebp-34h]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-3Ch]
        sub eax, dword ptr [ebp-14h]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-18h]
        sub eax, dword ptr [ebp-38h]
        shl eax, 1
        adc dh, dh
        mov eax, dword ptr [ebp-40h]
        sub eax, dword ptr [ebp-18h]
        shl eax, 1
        adc dh, dh
        mov dword ptr [ebp-1Ch], edx
        test dl, dh
        jne L403B1C
        or dl, dh
        jne L4037DA
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+4]
        sub dword ptr [ebp+14h], eax
        mov eax, dword ptr [esi+8]
        sub dword ptr [ebp+18h], eax
        push dword ptr [ebp-48h]
        push dword ptr [ebp+18h]
        push dword ptr [ebp+14h]
        push dword ptr [ebp-30h]
        push dword ptr [ebp+8]
        call DrawRLEImageColorUnclipped
        add esp, 14h
        jmp L403B14
L4037DA:
        mov eax, dword ptr [ebp-10h]
        imul dword ptr [ebp-48h]
        add eax, dword ptr [ebp-44h]
        add eax, dword ptr [ebp-0Ch]
        mov edi, eax
        mov ecx, dword ptr [ebp-10h]
        mov dword ptr [ebp-20h], ecx
        jmp L403806
L4037F0:
        movzx eax, al
        add esi, eax
        dec esi
L4037F6:
        inc esi
L4037F7:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4037F6
        jne L4037F0
        jb L4037F6
        add edi, dword ptr [ebp-48h]
        inc ecx
L403806:
        cmp ecx, dword ptr [ebp-38h]
        jl L4037F7
        mov dword ptr [ebp-24h], edi
        mov dword ptr [ebp-20h], ecx
        mov eax, edi
        sub eax, dword ptr [ebp-0Ch]
        add eax, dword ptr [ebp-34h]
        mov dword ptr [ebp-28h], eax
        mov eax, edi
        sub eax, dword ptr [ebp-0Ch]
        add eax, dword ptr [ebp-3Ch]
        mov dword ptr [ebp-2Ch], eax
        jmp L403B08
L40382C:
        mov eax, dword ptr [ebp-20h]
        cmp eax, dword ptr [ebp-40h]
        jg L403B14
        mov edi, dword ptr [ebp-24h]
        test dword ptr [ebp-1Ch], 8
        jne L403921
        test dword ptr [ebp-1Ch], 400h
        jne L4039AF
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40387D
        jne L4038AE
        jae L40391C
L403864:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40387D
        jne L4038AE
        jb L403864
        jae L40391C
L40387D:
        movzx ecx, al
L403880:
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40387D
        jae L40391C
        je L403864
L4038AE:
        movzx ecx, al
L4038B1:
        xor eax, eax
        or ecx, ecx
        je L403909
        cmp ecx, 4
        jl L4038FA
L4038BC:
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi], al
        mov al, byte ptr [esi+1]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+1], al
        mov al, byte ptr [esi+2]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+2], al
        mov al, byte ptr [esi+3]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+3], al
        add esi, 4
        add edi, 4
        sub ecx, 4
        je L403909
        cmp ecx, 4
        jge L4038BC
L4038FA:
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi], al
        inc esi
        inc edi
        dec ecx
        jne L4038FA
L403909:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L40387D
        jne L4038AE
        jb L403864
L40391C:
        jmp L403AF9
L403921:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403941
        jne L403972
        jae L4039AA
L40392C:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403941
        jne L403972
        jb L40392C
        jae L4039AA
L403941:
        movzx ecx, al
        mov eax, dword ptr [ebp-28h]
        sub eax, edi
        cmp eax, ecx
        jge L403964
        or eax, eax
        js L403955
        add edi, eax
        sub ecx, eax
L403955:
        test dword ptr [ebp-1Ch], 400h
        je L403880
        jne L4039DA
L403964:
        add edi, ecx
        inc esi
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403941
        jae L4039AA
        je L40392C
L403972:
        movzx ecx, al
        mov eax, dword ptr [ebp-28h]
        sub eax, edi
        cmp eax, ecx
        jge L40399B
        or eax, eax
        js L403988
        add edi, eax
        sub ecx, eax
        add esi, eax
L403988:
        test dword ptr [ebp-1Ch], 400h
        je L4038B1
        jne L403A29
L40399B:
        add edi, ecx
        add esi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403941
        jne L403972
        jb L40392C
L4039AA:
        jmp L403AF9
L4039AF:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4039D7
        jne L403A26
        jae L403AB4
L4039BE:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4039D7
        jne L403A26
        jb L4039BE
        jae L403AB4
L4039D7:
        movzx ecx, al
L4039DA:
        cmp edi, dword ptr [ebp-2Ch]
        jg L403AD9
        mov eax, edi
        add eax, ecx
        dec eax
        sub eax, dword ptr [ebp-2Ch]
        cdq
        not edx
        and edx, eax
        sub ecx, edx
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        add edi, edx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4039D7
        jae L403AB4
        je L4039BE
L403A26:
        movzx ecx, al
L403A29:
        cmp edi, dword ptr [ebp-2Ch]
        jg L403AEA
        mov eax, edi
        add eax, ecx
        dec eax
        sub eax, dword ptr [ebp-2Ch]
        cdq
        not edx
        and edx, eax
        sub ecx, edx
        xor eax, eax
        or ecx, ecx
        je L403A99
        cmp ecx, 4
        jl L403A8A
L403A4C:
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi], al
        mov al, byte ptr [esi+1]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+1], al
        mov al, byte ptr [esi+2]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+2], al
        mov al, byte ptr [esi+3]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+3], al
        add esi, 4
        add edi, 4
        sub ecx, 4
        je L403A99
        cmp ecx, 4
        jge L403A4C
L403A8A:
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi], al
        inc esi
        inc edi
        dec ecx
        jne L403A8A
L403A99:
        add edi, edx
        add esi, edx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4039D7
        jne L403A26
        jb L4039BE
L403AB4:
        jmp L403AF9
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403AD6
        jne L403AE7
        jae L403AF9
L403AC1:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403AD6
        jne L403AE7
        jb L403AC1
        jae L403AF9
L403AD6:
        movzx ecx, al
L403AD9:
        add edi, ecx
        inc esi
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403AD6
        jae L403AF9
        je L403AC1
L403AE7:
        movzx ecx, al
L403AEA:
        add edi, ecx
        add esi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403AD6
        jne L403AE7
        jb L403AC1
L403AF9:
        mov eax, dword ptr [ebp-48h]
        add dword ptr [ebp-24h], eax
        add dword ptr [ebp-28h], eax
        add dword ptr [ebp-2Ch], eax
        inc dword ptr [ebp-20h]
L403B08:
        mov eax, dword ptr [ebp-20h]
        cmp eax, dword ptr [ebp-18h]
        jle L40382C
L403B14:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L403B1C:
        mov eax, 0FFFFFFFDh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L403B27:
        mov eax, 0FFFFFFFCh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DrawRLEImageColor endp

; Function start: 0x00403B32
; int DrawRLEImageColorUnclipped(RasterClip *clip, RLEFrameHeader
;                                *frameHeader, int x, int y, int
;                                strideScratch);
; Draws an unclipped RLE frame through the colour translation table.
DrawRLEImageColorUnclipped proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+4]
        add dword ptr [ebp+10h], eax
        mov eax, dword ptr [esi+8]
        add dword ptr [ebp+14h], eax
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp+18h], eax
        jle L403C5D
        mov esi, dword ptr [ebp+0Ch]
        mov edi, dword ptr [ebx]
        mov eax, dword ptr [esi+8]
        add eax, dword ptr [ebp+10h]
        add edi, eax
        mov eax, dword ptr [esi+0Ch]
        mov ebx, eax
        add eax, dword ptr [ebp+14h]
        mul dword ptr [ebp+18h]
        add edi, eax
        mov edx, edi
        mov eax, dword ptr [esi+10h]
        mov eax, dword ptr [esi+14h]
        inc eax
        sub eax, ebx
        mov ebx, eax
        jle L403C5D
        add esi, 18h
L403B8A:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403BB2
        jne L403BE3
        jae L403C51
L403B99:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403BB2
        jne L403BE3
        jb L403B99
        jae L403C51
L403BB2:
        movzx ecx, al
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403BB2
        jae L403C51
        je L403B99
L403BE3:
        movzx ecx, al
        xor eax, eax
        or ecx, ecx
        je L403C3E
        cmp ecx, 4
        jl L403C2F
L403BF1:
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi], al
        mov al, byte ptr [esi+1]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+1], al
        mov al, byte ptr [esi+2]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+2], al
        mov al, byte ptr [esi+3]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi+3], al
        add esi, 4
        add edi, 4
        sub ecx, 4
        je L403C3E
        cmp ecx, 4
        jge L403BF1
L403C2F:
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [edi], al
        inc esi
        inc edi
        dec ecx
        jne L403C2F
L403C3E:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L403BB2
        jne L403BE3
        jb L403B99
L403C51:
        add edx, dword ptr [ebp+18h]
        mov edi, edx
        dec ebx
        jne L403B8A
L403C5D:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DrawRLEImageColorUnclipped endp

; Function start: 0x00403C65
; int RotateRLEImage(RasterClip *clip, unsigned char *shape, int frame, int x,
;                    int y, unsigned char *scratch, unsigned int angleTenths,
;                    int scaleX, int scaleY, unsigned int flags);
; Draws an RLE frame rotated and scaled through the transform vertices.
RotateRLEImage proc
        push ebp
        mov ebp, esp
        add esp, 0FFFFFF10h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        cmp dword ptr [ebp+24h], 10000h
        jne L403C91
        cmp dword ptr [ebp+28h], 10000h
        jne L403C91
        cmp dword ptr [ebp+20h], 0
        je L4047CF
L403C91:
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        call GetRLEImageSize
        add esp, 8
        mov ecx, eax
        shr eax, 10h
        dec eax
        mov dword ptr [ebp-44h], eax
        and ecx, 0FFFFh
        dec ecx
        mov dword ptr [ebp-48h], ecx
        lea ebx, dword ptr [ebp-14h]
        lea esi, dword ptr [ebp-28h]
        mov dword ptr [esi], ebx
        mov edx, dword ptr [ebp+1Ch]
        mov dword ptr [ebx], edx
        mov dword ptr [esi+4], 0
        mov dword ptr [g_aRLETransformVertices_00491c08+0Ch], 0
        mov dword ptr [g_aRLETransformVertices_00491c08+48h], 0
        mov dword ptr [esi+8], 0
        mov dword ptr [g_aRLETransformVertices_00491c08+10h], 0
        mov dword ptr [g_aRLETransformVertices_00491c08+24h], 0
        mov dword ptr [ebx+4], eax
        mov dword ptr [esi+0Ch], eax
        mov dword ptr [g_aRLETransformVertices_00491c08+20h], eax
        mov dword ptr [g_aRLETransformVertices_00491c08+34h], eax
        mov dword ptr [ebx+8], ecx
        mov dword ptr [esi+10h], ecx
        mov dword ptr [g_aRLETransformVertices_00491c08+38h], ecx
        mov dword ptr [g_aRLETransformVertices_00491c08+4Ch], ecx
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        call GetRLEImageOrigin
        add esp, 8
        mov ebx, eax
        cwde
        neg eax
        mov dword ptr [ebp-3Ch], eax
        sar ebx, 10h
        neg ebx
        mov dword ptr [ebp-40h], ebx
        test dword ptr [ebp+2Ch], 2
        jne L403D8A
        push 0FFh
        lea eax, dword ptr [ebp-28h]
        push eax
        call FillRasterClip
        add esp, 8
        test dword ptr [ebp+2Ch], 1
        je L403D72
        push dword ptr [ebp-3Ch]
        push dword ptr [ebp-40h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        lea eax, dword ptr [ebp-28h]
        push eax
        call DrawRLEImageColor
        add esp, 14h
        jmp L403D8A
L403D72:
        push dword ptr [ebp-3Ch]
        push dword ptr [ebp-40h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        lea eax, dword ptr [ebp-28h]
        push eax
        call DrawRLEImage
        add esp, 14h
L403D8A:
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-0E8h], eax
        jle L403E2B
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L403E2B
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-0ECh], eax
        cmp eax, 0
        jg L403DBE
        mov eax, 0
L403DBE:
        mov dword ptr [ebp-0D4h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-0F0h], eax
        cmp eax, 0
        jg L403DD7
        mov eax, 0
L403DD7:
        mov dword ptr [ebp-0D8h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-0E8h]
        dec edx
        cmp eax, edx
        jl L403DED
        mov eax, edx
L403DED:
        mov dword ptr [ebp-0DCh], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L403DFF
        mov eax, edx
L403DFF:
        mov dword ptr [ebp-0E0h], eax
        mov eax, dword ptr [ebp-0DCh]
        cmp eax, dword ptr [ebp-0D4h]
        jl L403E36
        mov eax, dword ptr [ebp-0E0h]
        cmp eax, dword ptr [ebp-0D8h]
        jl L403E36
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-0E4h], eax
        jmp L403E41
L403E2B:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L403E36:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L403E41:
        mov eax, dword ptr [ebp+14h]
        sub eax, dword ptr [ebp-40h]
        mov dword ptr [ebp-4Ch], eax
        mov eax, dword ptr [ebp+18h]
        sub eax, dword ptr [ebp-3Ch]
        mov dword ptr [ebp-50h], eax
        mov dword ptr [ebp-30h], 0
        mov dword ptr [ebp-2Ch], 0
        push dword ptr [ebp+28h]
        push dword ptr [ebp+24h]
        push dword ptr [ebp+20h]
        lea eax, dword ptr [ebp-40h]
        push eax
        lea eax, dword ptr [ebp-38h]
        push eax
        lea eax, dword ptr [ebp-30h]
        push eax
        call TransformRLEPoint
        add esp, 18h
        mov eax, dword ptr [ebp-38h]
        add eax, dword ptr [ebp-4Ch]
        mov dword ptr [g_aRLETransformVertices_00491c08], eax
        mov eax, dword ptr [ebp-34h]
        add eax, dword ptr [ebp-50h]
        mov dword ptr [g_aRLETransformVertices_00491c08+4], eax
        mov eax, dword ptr [ebp-44h]
        mov dword ptr [ebp-30h], eax
        mov dword ptr [ebp-2Ch], 0
        push dword ptr [ebp+28h]
        push dword ptr [ebp+24h]
        push dword ptr [ebp+20h]
        lea eax, dword ptr [ebp-40h]
        push eax
        lea eax, dword ptr [ebp-38h]
        push eax
        lea eax, dword ptr [ebp-30h]
        push eax
        call TransformRLEPoint
        add esp, 18h
        mov eax, dword ptr [ebp-38h]
        add eax, dword ptr [ebp-4Ch]
        mov dword ptr [g_aRLETransformVertices_00491c08+14h], eax
        mov eax, dword ptr [ebp-34h]
        add eax, dword ptr [ebp-50h]
        mov dword ptr [g_aRLETransformVertices_00491c08+18h], eax
        mov eax, dword ptr [ebp-0ECh]
        add dword ptr [g_aRLETransformVertices_00491c08], eax
        add dword ptr [g_aRLETransformVertices_00491c08+14h], eax
        mov eax, dword ptr [ebp-0F0h]
        add dword ptr [g_aRLETransformVertices_00491c08+4], eax
        add dword ptr [g_aRLETransformVertices_00491c08+18h], eax
        mov eax, dword ptr [ebp-44h]
        mov ebx, dword ptr [ebp-48h]
        mov dword ptr [ebp-30h], eax
        mov dword ptr [ebp-2Ch], ebx
        push dword ptr [ebp+28h]
        push dword ptr [ebp+24h]
        push dword ptr [ebp+20h]
        lea eax, dword ptr [ebp-40h]
        push eax
        lea eax, dword ptr [ebp-38h]
        push eax
        lea eax, dword ptr [ebp-30h]
        push eax
        call TransformRLEPoint
        add esp, 18h
        mov eax, dword ptr [ebp-38h]
        add eax, dword ptr [ebp-4Ch]
        mov dword ptr [g_aRLETransformVertices_00491c08+28h], eax
        mov eax, dword ptr [ebp-34h]
        add eax, dword ptr [ebp-50h]
        mov dword ptr [g_aRLETransformVertices_00491c08+2Ch], eax
        mov eax, dword ptr [ebp-48h]
        mov dword ptr [ebp-30h], 0
        mov dword ptr [ebp-2Ch], eax
        push dword ptr [ebp+28h]
        push dword ptr [ebp+24h]
        push dword ptr [ebp+20h]
        lea eax, dword ptr [ebp-40h]
        push eax
        lea eax, dword ptr [ebp-38h]
        push eax
        lea eax, dword ptr [ebp-30h]
        push eax
        call TransformRLEPoint
        add esp, 18h
        mov eax, dword ptr [ebp-38h]
        add eax, dword ptr [ebp-4Ch]
        mov dword ptr [g_aRLETransformVertices_00491c08+3Ch], eax
        mov eax, dword ptr [ebp-34h]
        add eax, dword ptr [ebp-50h]
        mov dword ptr [g_aRLETransformVertices_00491c08+40h], eax
        mov eax, dword ptr [ebp-0ECh]
        add dword ptr [g_aRLETransformVertices_00491c08+28h], eax
        add dword ptr [g_aRLETransformVertices_00491c08+3Ch], eax
        mov eax, dword ptr [ebp-0F0h]
        add dword ptr [g_aRLETransformVertices_00491c08+2Ch], eax
        add dword ptr [g_aRLETransformVertices_00491c08+40h], eax
        lea ebx, dword ptr [ebp-14h]
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-58h], eax
        mov ecx, dword ptr [ebx+4]
        inc ecx
        mov dword ptr [ebp-54h], ecx
        push ds
        pop es
        mov ebx, offset g_aRLETransformVertices_00491c08
        mov eax, ebx
        add eax, 50h
        mov dword ptr [ebp-64h], ebx
        mov dword ptr [ebp-68h], eax
        mov esi, 7FFFh
        mov edi, 0FFFF8000h
        mov ecx, 0Fh
L403FCB:
        mov edx, 0
        mov eax, dword ptr [ebx]
        sub eax, dword ptr [ebp-0D4h]
        shld edx, eax, 1
        mov eax, dword ptr [ebp-0DCh]
        sub eax, dword ptr [ebx]
        shld edx, eax, 1
        mov eax, dword ptr [ebx+4]
        sub eax, dword ptr [ebp-0D8h]
        shld edx, eax, 1
        mov eax, dword ptr [ebp-0E0h]
        sub eax, dword ptr [ebx+4]
        shld edx, eax, 1
        mov eax, dword ptr [ebx+4]
        cmp eax, esi
        jg L40400E
        mov esi, eax
        mov dword ptr [ebp-6Ch], ebx
L40400E:
        cmp eax, edi
        jl L404014
        mov edi, eax
L404014:
        and ecx, edx
        add ebx, 14h
        cmp ebx, dword ptr [ebp-68h]
        jne L403FCB
        or ecx, ecx
        jne L404600
        mov eax, dword ptr [ebp-6Ch]
        mov dword ptr [ebp-78h], eax
        mov dword ptr [ebp-7Ch], eax
        mov dword ptr [ebp-8Ch], esi
        cmp edi, esi
        je L404600
L40403D:
        mov ebx, dword ptr [ebp-78h]
        mov dword ptr [ebp-70h], ebx
        mov esi, ebx
        sub esi, 14h
        cmp esi, dword ptr [ebp-64h]
        jge L404053
        mov esi, dword ptr [ebp-68h]
        sub esi, 14h
L404053:
        mov dword ptr [ebp-78h], esi
        mov ecx, dword ptr [esi+4]
        mov edx, dword ptr [ebx+4]
        cmp edx, dword ptr [ebp-0D8h]
        jge L40406C
        cmp ecx, dword ptr [ebp-0D8h]
        jle L40403D
L40406C:
        sub ecx, edx
        je L40403D
        mov dword ptr [ebp-80h], ecx
        mov edx, dword ptr [esi]
        sub edx, dword ptr [ebx]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0A8h], eax
        mov ecx, dword ptr [ebp-80h]
        mov edx, dword ptr [esi+0Ch]
        sub edx, dword ptr [ebx+0Ch]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0B0h], eax
        mov ecx, dword ptr [ebp-80h]
        mov edx, dword ptr [esi+10h]
        sub edx, dword ptr [ebx+10h]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0B8h], eax
        mov edx, dword ptr [ebx]
        shl edx, 10h
        add edx, 8000h
        mov dword ptr [ebp-90h], edx
        mov edx, dword ptr [ebx+0Ch]
        shl edx, 10h
        add edx, 8000h
        mov dword ptr [ebp-98h], edx
        mov edx, dword ptr [ebx+10h]
        shl edx, 10h
        add edx, 8000h
        mov dword ptr [ebp-0A0h], edx
L404103:
        mov ebx, dword ptr [ebp-7Ch]
        mov dword ptr [ebp-74h], ebx
        mov esi, ebx
        add esi, 14h
        cmp esi, dword ptr [ebp-68h]
        jl L404116
        mov esi, dword ptr [ebp-64h]
L404116:
        mov dword ptr [ebp-7Ch], esi
        mov ecx, dword ptr [esi+4]
        mov edx, dword ptr [ebx+4]
        cmp edx, dword ptr [ebp-0D8h]
        jge L40412F
        cmp ecx, dword ptr [ebp-0D8h]
        jle L404103
L40412F:
        sub ecx, edx
        je L404103
        mov dword ptr [ebp-84h], ecx
        mov edx, dword ptr [esi]
        sub edx, dword ptr [ebx]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0ACh], eax
        mov ecx, dword ptr [ebp-84h]
        mov edx, dword ptr [esi+0Ch]
        sub edx, dword ptr [ebx+0Ch]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0B4h], eax
        mov ecx, dword ptr [ebp-84h]
        mov edx, dword ptr [esi+10h]
        sub edx, dword ptr [ebx+10h]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0BCh], eax
        mov edx, dword ptr [ebx]
        shl edx, 10h
        add edx, 8000h
        mov dword ptr [ebp-94h], edx
        mov edx, dword ptr [ebx+0Ch]
        shl edx, 10h
        add edx, 8000h
        mov dword ptr [ebp-9Ch], edx
        mov edx, dword ptr [ebx+10h]
        shl edx, 10h
        add edx, 8000h
        mov dword ptr [ebp-0A4h], edx
        mov eax, dword ptr [ebp-0E0h]
        sub eax, dword ptr [ebp-8Ch]
        sub edi, dword ptr [ebp-0E0h]
        jg L4041E5
        add eax, edi
L4041E5:
        mov dword ptr [ebp-88h], eax
        mov eax, dword ptr [ebp-0D8h]
        sub eax, dword ptr [ebp-8Ch]
        jle L40429C
        sub dword ptr [ebp-88h], eax
        mov ecx, dword ptr [ebp-0D8h]
        mov dword ptr [ebp-8Ch], ecx
        mov ebx, dword ptr [ebp-70h]
        sub ecx, dword ptr [ebx+4]
        sub dword ptr [ebp-80h], ecx
        shl ecx, 10h
        mov eax, dword ptr [ebp-0A8h]
        imul ecx
        shrd eax, edx, 10h
        add dword ptr [ebp-90h], eax
        mov eax, dword ptr [ebp-0B0h]
        imul ecx
        shrd eax, edx, 10h
        add dword ptr [ebp-98h], eax
        mov eax, dword ptr [ebp-0B8h]
        imul ecx
        shrd eax, edx, 10h
        add dword ptr [ebp-0A0h], eax
        mov ecx, dword ptr [ebp-0D8h]
        mov ebx, dword ptr [ebp-74h]
        sub ecx, dword ptr [ebx+4]
        sub dword ptr [ebp-84h], ecx
        shl ecx, 10h
        mov eax, dword ptr [ebp-0ACh]
        imul ecx
        shrd eax, edx, 10h
        add dword ptr [ebp-94h], eax
        mov eax, dword ptr [ebp-0B4h]
        imul ecx
        shrd eax, edx, 10h
        add dword ptr [ebp-9Ch], eax
        mov eax, dword ptr [ebp-0BCh]
        imul ecx
        shrd eax, edx, 10h
        add dword ptr [ebp-0A4h], eax
L40429C:
        mov eax, dword ptr [ebp-8Ch]
        imul dword ptr [ebp-0E8h]
        add eax, dword ptr [ebp-0E4h]
        add eax, 0
        mov dword ptr [ebp-5Ch], eax
        mov eax, dword ptr [ebp-90h]
        mov ebx, dword ptr [ebp-94h]
        mov ecx, dword ptr [ebp-98h]
        mov edx, dword ptr [ebp-9Ch]
        mov esi, dword ptr [ebp-0A0h]
        mov edi, dword ptr [ebp-0A4h]
L4042D8:
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        cmp ebx, eax
        jg L4042E7
        xchg eax, ebx
        xchg ecx, edx
        xchg esi, edi
L4042E7:
        sar eax, 10h
        cmp eax, dword ptr [ebp-0DCh]
        jg L4045A9
        sar ebx, 10h
        cmp ebx, dword ptr [ebp-0D4h]
        jl L4045A9
        mov dword ptr [ebp-0C0h], eax
        mov dword ptr [ebp-0C4h], ebx
        mov dword ptr [ebp-0D0h], ecx
        sub ebx, eax
        je L4043EC
        push ebx
        sub edx, ecx
        xor eax, eax
        shrd eax, edx, 10h
        shl ebx, 10h
        sar edx, 10h
        idiv ebx
        mov dword ptr [ebp-0C8h], eax
        shld edx, eax, 10h
        pop ebx
        and eax, 0FFFFh
        and edx, 0FFFFh
        mov ecx, 1
        test edx, 8000h
        je L404361
        or edx, 0FFFF0000h
        neg ecx
        cmp eax, 1
        sbb edx, -1
L404361:
        add ecx, edx
        push ecx
        push edx
        sub edi, esi
        mov edx, edi
        xor eax, eax
        shrd eax, edx, 10h
        shl ebx, 10h
        sar edx, 10h
        idiv ebx
        mov dword ptr [ebp-0CCh], eax
        shld edx, eax, 10h
        and eax, 0FFFFh
        and edx, 0FFFFh
        mov ecx, dword ptr [ebp-54h]
        test edx, 8000h
        je L40439F
        neg ecx
        cmp eax, 1
        sbb edx, -1
L40439F:
        mov eax, dword ptr [ebp-54h]
        imul dx
        cwde
        pop edx
        pop ebx
        add edx, eax
        mov dword ptr [g_anRLESourceSteps_00491c58], edx
        add edx, ecx
        mov dword ptr [g_anRLESourceSteps_00491c58+4], edx
        add ebx, eax
        mov dword ptr [g_anRLESourceSteps_00491c58+8], ebx
        add ebx, ecx
        mov dword ptr [g_anRLESourceSteps_00491c58+0Ch], ebx
        mov ecx, dword ptr [ebp-0D4h]
        sub ecx, dword ptr [ebp-0C0h]
        jg L40462F
L4043DA:
        mov eax, dword ptr [ebp-0C4h]
        sub eax, dword ptr [ebp-0DCh]
        jg L40465D
L4043EC:
        mov ecx, esi
        shr esi, 10h
        mov eax, esi
        mul dword ptr [ebp-54h]
        add eax, dword ptr [ebp-58h]
        mov esi, dword ptr [ebp-0D0h]
        shr esi, 10h
        add esi, eax
        mov eax, dword ptr [ebp-0C0h]
        mov edi, dword ptr [ebp-5Ch]
        add edi, eax
        mov ebx, dword ptr [ebp-0C4h]
        sub ebx, eax
        push ebp
        mov edx, dword ptr [ebp-0D0h]
        mov eax, dword ptr [ebp-0C8h]
        or eax, eax
        jns L40442C
        neg eax
        not edx
L40442C:
        shl eax, 10h
        shl edx, 10h
        mov ebp, dword ptr [ebp-0CCh]
        or ebp, ebp
        jns L404440
        neg ebp
        not ecx
L404440:
        shl ebp, 10h
        shl ecx, 10h
        push ebx
        xor ebx, ebx
        cmp dword ptr [esp], 5
        jl L40450B
L404453:
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L40445C
        mov byte ptr [edi], bl
L40445C:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L404477
        mov byte ptr [edi+1], bl
L404477:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L404492
        mov byte ptr [edi+2], bl
L404492:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L4044AD
        mov byte ptr [edi+3], bl
L4044AD:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L4044C8
        mov byte ptr [edi+4], bl
L4044C8:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L4044E3
        mov byte ptr [edi+5], bl
L4044E3:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        add edi, 6
        sub dword ptr [esp], 6
        js L4045A5
        cmp dword ptr [esp], 5
        jge L404453
L40450B:
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L404514
        mov byte ptr [edi], bl
L404514:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        dec dword ptr [esp]
        js L4045A5
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L404534
        mov byte ptr [edi+1], bl
L404534:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        dec dword ptr [esp]
        js L4045A5
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L404554
        mov byte ptr [edi+2], bl
L404554:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        dec dword ptr [esp]
        js L4045A5
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L404574
        mov byte ptr [edi+3], bl
L404574:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
        dec dword ptr [esp]
        js L4045A5
        mov bl, byte ptr [esi]
        cmp bl, 0FFh
        je L404594
        mov byte ptr [edi+4], bl
L404594:
        xor ebx, ebx
        add edx, eax
        adc ebx, ebx
        add ecx, ebp
        adc ebx, ebx
        add esi, dword ptr [ebx*4+g_anRLESourceSteps_00491c58]
L4045A5:
        add esp, 4
        pop ebp
L4045A9:
        mov edi, dword ptr [ebp-0E8h]
        add dword ptr [ebp-5Ch], edi
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        dec dword ptr [ebp-88h]
        js L404600
        je L404606
        dec dword ptr [ebp-80h]
        je L404668
        add eax, dword ptr [ebp-0A8h]
        add ecx, dword ptr [ebp-0B0h]
        add esi, dword ptr [ebp-0B8h]
L4045DD:
        dec dword ptr [ebp-84h]
        je L404718
        add ebx, dword ptr [ebp-0ACh]
        add edx, dword ptr [ebp-0B4h]
        add edi, dword ptr [ebp-0BCh]
        jmp L4042D8
L404600:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L404606:
        add eax, dword ptr [ebp-0A8h]
        add ecx, dword ptr [ebp-0B0h]
        add esi, dword ptr [ebp-0B8h]
        add ebx, dword ptr [ebp-0ACh]
        add edx, dword ptr [ebp-0B4h]
        add edi, dword ptr [ebp-0BCh]
        jmp L4042D8
L40462F:
        add dword ptr [ebp-0C0h], ecx
        shl ecx, 10h
        mov eax, dword ptr [ebp-0C8h]
        imul ecx
        shrd eax, edx, 10h
        add dword ptr [ebp-0D0h], eax
        mov eax, dword ptr [ebp-0CCh]
        imul ecx
        shrd eax, edx, 10h
        add esi, eax
        jmp L4043DA
L40465D:
        sub dword ptr [ebp-0C4h], eax
        jmp L4043EC
L404668:
        push ebx
        push edx
        mov ebx, dword ptr [ebp-78h]
        mov dword ptr [ebp-70h], ebx
        mov esi, ebx
        sub esi, 14h
        cmp esi, dword ptr [ebp-64h]
        jge L404680
        mov esi, dword ptr [ebp-68h]
        sub esi, 14h
L404680:
        mov dword ptr [ebp-78h], esi
        mov ecx, dword ptr [esi+4]
        mov edx, dword ptr [ebx+4]
        sub ecx, edx
        cmp ecx, 1
        adc ecx, 0
        mov dword ptr [ebp-80h], ecx
        mov edx, dword ptr [esi]
        sub edx, dword ptr [ebx]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0A8h], eax
        mov ecx, dword ptr [ebp-80h]
        mov edx, dword ptr [esi+0Ch]
        sub edx, dword ptr [ebx+0Ch]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0B0h], eax
        mov ecx, dword ptr [ebp-80h]
        mov edx, dword ptr [esi+10h]
        sub edx, dword ptr [ebx+10h]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0B8h], eax
        mov eax, dword ptr [ebx]
        shl eax, 10h
        add eax, 8000h
        mov ecx, dword ptr [ebx+0Ch]
        shl ecx, 10h
        add ecx, 8000h
        mov esi, dword ptr [ebx+10h]
        shl esi, 10h
        add esi, 8000h
        pop edx
        pop ebx
        jmp L4045DD
L404718:
        push eax
        push ecx
        mov ebx, dword ptr [ebp-7Ch]
        mov dword ptr [ebp-74h], ebx
        mov edi, ebx
        add edi, 14h
        cmp edi, dword ptr [ebp-68h]
        jl L40472D
        mov edi, dword ptr [ebp-64h]
L40472D:
        mov dword ptr [ebp-7Ch], edi
        mov ecx, dword ptr [edi+4]
        mov edx, dword ptr [ebx+4]
        sub ecx, edx
        cmp ecx, 1
        adc ecx, 0
        mov dword ptr [ebp-84h], ecx
        mov edx, dword ptr [edi]
        sub edx, dword ptr [ebx]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0ACh], eax
        mov ecx, dword ptr [ebp-84h]
        mov edx, dword ptr [edi+0Ch]
        sub edx, dword ptr [ebx+0Ch]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0B4h], eax
        mov ecx, dword ptr [ebp-84h]
        mov edx, dword ptr [edi+10h]
        sub edx, dword ptr [ebx+10h]
        shl edx, 10h
        xor eax, eax
        shrd eax, edx, 10h
        shl ecx, 10h
        sar edx, 10h
        idiv ecx
        mov dword ptr [ebp-0BCh], eax
        mov edx, dword ptr [ebx+0Ch]
        shl edx, 10h
        add edx, 8000h
        mov edi, dword ptr [ebx+10h]
        shl edi, 10h
        add edi, 8000h
        mov ebx, dword ptr [ebx]
        shl ebx, 10h
        add ebx, 8000h
        pop ecx
        pop eax
        jmp L4042D8
L4047CF:
        test dword ptr [ebp+2Ch], 1
        je L4047F4
        push dword ptr [ebp+18h]
        push dword ptr [ebp+14h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push dword ptr [ebp+8]
        call DrawRLEImageColor
        add esp, 14h
        jmp L404600
L4047F4:
        push dword ptr [ebp+18h]
        push dword ptr [ebp+14h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push dword ptr [ebp+8]
        call DrawRLEImage
        add esp, 14h
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
RotateRLEImage endp

; Function start: 0x00404811
; unsigned int GetRLEFrameBounds(unsigned char *shape, int frame, int x, int
;                                y, unsigned int flags, int *bounds);
; Measures the bounds of an RLE frame.
GetRLEFrameBounds proc
        push ebp
        mov ebp, esp
        add esp, -24h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov dword ptr [ebp-18h], 0
        mov dword ptr [ebp-1Ch], 0
        mov dword ptr [ebp-20h], 0
        mov dword ptr [ebp-24h], 0
        mov esi, dword ptr [ebp+0Ch]
        shl esi, 3
        add esi, 8
        add esi, dword ptr [ebp+8]
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+8]
        mov dword ptr [ebp-4], esi
        mov esi, dword ptr [ebp-4]
        mov eax, dword ptr [esi+8]
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [esi+0Ch]
        mov ebx, eax
        add eax, dword ptr [ebp+14h]
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+10h]
        add eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [esi+14h]
        mov ecx, eax
        add ecx, dword ptr [ebp+14h]
        mov dword ptr [ebp-14h], ecx
        inc eax
        sub eax, ebx
        mov ebx, eax
        jle L404936
        add esi, 18h
        mov dword ptr [ebp-18h], 7FFFFFFFh
        mov dword ptr [ebp-1Ch], 7FFFFFFFh
        mov dword ptr [ebp-20h], 80000000h
        mov dword ptr [ebp-24h], 80000000h
        mov edx, dword ptr [ebp-0Ch]
L4048A6:
        mov edi, dword ptr [ebp-8]
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4048C9
        jne L4048FC
        jae L40492E
L4048B4:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4048C9
        jne L4048FC
        jb L4048B4
        jae L40492E
L4048C9:
        movzx ecx, al
        mov al, byte ptr [esi]
        inc esi
        cmp dword ptr [ebp-18h], edi
        jl L4048D7
        mov dword ptr [ebp-18h], edi
L4048D7:
        add edi, ecx
        cmp dword ptr [ebp-20h], edi
        jg L4048E1
        mov dword ptr [ebp-20h], edi
L4048E1:
        cmp dword ptr [ebp-1Ch], edx
        jl L4048E9
        mov dword ptr [ebp-1Ch], edx
L4048E9:
        cmp dword ptr [ebp-24h], edx
        jg L4048F1
        mov dword ptr [ebp-24h], edx
L4048F1:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4048C9
        jae L40492E
        je L4048B4
L4048FC:
        movzx ecx, al
        add esi, ecx
        cmp dword ptr [ebp-18h], edi
        jl L404909
        mov dword ptr [ebp-18h], edi
L404909:
        add edi, ecx
        cmp dword ptr [ebp-20h], edi
        jg L404913
        mov dword ptr [ebp-20h], edi
L404913:
        cmp dword ptr [ebp-1Ch], edx
        jl L40491B
        mov dword ptr [ebp-1Ch], edx
L40491B:
        cmp dword ptr [ebp-24h], edx
        jg L404923
        mov dword ptr [ebp-24h], edx
L404923:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L4048C9
        jne L4048FC
        jb L4048B4
L40492E:
        inc edx
        dec ebx
        jne L4048A6
L404936:
        mov eax, dword ptr [ebp+18h]
        and eax, 1
        je L404952
        mov eax, dword ptr [ebp+10h]
        add eax, dword ptr [ebp+10h]
        mov ebx, eax
        sub eax, dword ptr [ebp-18h]
        sub ebx, dword ptr [ebp-20h]
        mov dword ptr [ebp-20h], eax
        mov dword ptr [ebp-18h], ebx
L404952:
        mov eax, dword ptr [ebp+18h]
        and eax, 2
        je L40496E
        mov eax, dword ptr [ebp+14h]
        add eax, dword ptr [ebp+14h]
        mov ebx, eax
        sub eax, dword ptr [ebp-1Ch]
        sub ebx, dword ptr [ebp-24h]
        mov dword ptr [ebp-24h], eax
        mov dword ptr [ebp-1Ch], ebx
L40496E:
        mov edi, dword ptr [ebp+1Ch]
        mov eax, dword ptr [ebp-18h]
        stosd
        mov eax, dword ptr [ebp-1Ch]
        stosd
        mov eax, dword ptr [ebp-20h]
        stosd
        mov eax, dword ptr [ebp-24h]
        stosd
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRLEFrameBounds endp

; Function start: 0x00404989
; int EncodeRasterClipToRLEFrame(RasterClip *clip, unsigned char
;                                transparentColour, int originX, int originY,
;                                unsigned char *output);
; Encodes a clip into an RLE frame.
EncodeRasterClipToRLEFrame proc
        push ebp
        mov ebp, esp
        add esp, -38h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-30h], eax
        jle L404A08
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L404A08
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-34h], eax
        cmp eax, 0
        jg L4049BC
        mov eax, 0
L4049BC:
        mov dword ptr [ebp-1Ch], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-38h], eax
        cmp eax, 0
        jg L4049CF
        mov eax, 0
L4049CF:
        mov dword ptr [ebp-20h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-30h]
        dec edx
        cmp eax, edx
        jl L4049DF
        mov eax, edx
L4049DF:
        mov dword ptr [ebp-24h], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L4049EE
        mov eax, edx
L4049EE:
        mov dword ptr [ebp-28h], eax
        mov eax, dword ptr [ebp-24h]
        cmp eax, dword ptr [ebp-1Ch]
        jl L404A13
        mov eax, dword ptr [ebp-28h]
        cmp eax, dword ptr [ebp-20h]
        jl L404A13
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-2Ch], eax
        jmp L404A1E
L404A08:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L404A13:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L404A1E:
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov edi, dword ptr [ebp+18h]
        mov dword ptr [g_pRLEEncodeBuffer_004902b9], edi
        or edi, edi
        je L404A6C
        mov eax, dword ptr [esi+0Ch]
        sub eax, dword ptr [esi+4]
        shl eax, 10h
        mov ax, word ptr [esi+10h]
        sub ax, word ptr [esi+8]
        mov dword ptr [edi], eax
        add edi, 4
        mov eax, dword ptr [ebp+10h]
        shl eax, 10h
        mov ax, word ptr [ebp+14h]
        mov dword ptr [edi], eax
        add edi, 4
        xor eax, eax
        mov dword ptr [edi], eax
        add edi, 4
        mov dword ptr [edi], eax
        add edi, 4
        dec eax
        mov dword ptr [edi], eax
        add edi, 4
        mov dword ptr [edi], eax
        add edi, 4
L404A6C:
        mov eax, dword ptr [ebp-34h]
        add dword ptr [ebp+10h], eax
        mov eax, dword ptr [ebp-38h]
        add dword ptr [ebp+14h], eax
        mov eax, dword ptr [ebp-24h]
        inc eax
        sub eax, dword ptr [ebp-1Ch]
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [ebp-24h]
        inc eax
        sub eax, dword ptr [ebp-1Ch]
        mov dword ptr [ebp-0Ch], eax
        mov eax, 7FFFFFFFh
        mov dword ptr [g_nRLEEncodedMinimumX_004902e1], eax
        mov dword ptr [g_nRLEEncodedMinimumY_004902e5], eax
        neg eax
        mov dword ptr [g_nRLEEncodedMaximumX_004902e9], eax
        mov dword ptr [g_nRLEEncodedMaximumY_004902ed], eax
        mov eax, dword ptr [ebp-20h]
        imul dword ptr [ebp-30h]
        add eax, dword ptr [ebp-2Ch]
        add eax, dword ptr [ebp-1Ch]
        mov dword ptr [g_pRLEScanlineStart_004902c1], eax
        mov esi, eax
        mov eax, dword ptr [ebp-20h]
        mov dword ptr [ebp-10h], eax
        jmp L404B5C
L404AC5:
        mov edi, dword ptr [g_pRLEScanlineStart_004902c1]
        mov dword ptr [g_pRLEScanCursor_004902c5], edi
        mov ecx, dword ptr [ebp-8]
        mov dword ptr [ebp-14h], ecx
        mov al, byte ptr [ebp+0Ch]
        repe scasb
        je L404B50
        mov eax, dword ptr [g_nRLEEncodedMinimumY_004902e5]
        cmp eax, dword ptr [ebp-10h]
        jl L404AEB
        mov eax, dword ptr [ebp-10h]
L404AEB:
        mov dword ptr [g_nRLEEncodedMinimumY_004902e5], eax
        mov eax, dword ptr [g_nRLEEncodedMaximumY_004902ed]
        cmp eax, dword ptr [ebp-10h]
        jg L404AFD
        mov eax, dword ptr [ebp-10h]
L404AFD:
        mov dword ptr [g_nRLEEncodedMaximumY_004902ed], eax
        mov eax, dword ptr [ebp-24h]
        sub eax, ecx
        cmp eax, dword ptr [g_nRLEEncodedMinimumX_004902e1]
        jl L404B14
        mov eax, dword ptr [g_nRLEEncodedMinimumX_004902e1]
L404B14:
        mov dword ptr [g_nRLEEncodedMinimumX_004902e1], eax
        mov eax, dword ptr [g_pRLEScanlineStart_004902c1]
        add eax, dword ptr [ebp-8]
        dec eax
        mov edi, eax
        mov dword ptr [g_pRLEScanCursor_004902c5], edi
        mov ecx, dword ptr [ebp-8]
        mov dword ptr [ebp-14h], ecx
        mov al, byte ptr [ebp+0Ch]
        std
        repe scasb
        cld
        je L404B50
        mov eax, dword ptr [ebp-1Ch]
        add eax, ecx
        cmp eax, dword ptr [g_nRLEEncodedMaximumX_004902e9]
        jg L404B4B
        mov eax, dword ptr [g_nRLEEncodedMaximumX_004902e9]
L404B4B:
        mov dword ptr [g_nRLEEncodedMaximumX_004902e9], eax
L404B50:
        mov eax, dword ptr [ebp-30h]
        add dword ptr [g_pRLEScanlineStart_004902c1], eax
        inc dword ptr [ebp-10h]
L404B5C:
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [ebp-28h]
        jle L404AC5
        mov edi, dword ptr [ebp+18h]
        or edi, edi
        je L404B9B
        mov eax, dword ptr [g_nRLEEncodedMinimumX_004902e1]
        sub eax, dword ptr [ebp+10h]
        mov dword ptr [edi+8], eax
        mov eax, dword ptr [g_nRLEEncodedMinimumY_004902e5]
        sub eax, dword ptr [ebp+14h]
        mov dword ptr [edi+0Ch], eax
        mov eax, dword ptr [g_nRLEEncodedMaximumX_004902e9]
        sub eax, dword ptr [ebp+10h]
        mov dword ptr [edi+10h], eax
        mov eax, dword ptr [g_nRLEEncodedMaximumY_004902ed]
        sub eax, dword ptr [ebp+14h]
        mov dword ptr [edi+14h], eax
L404B9B:
        add edi, 18h
        mov dword ptr [g_pRLEOutputCursor_004902c9], edi
        mov eax, dword ptr [g_nRLEEncodedMaximumX_004902e9]
        inc eax
        sub eax, dword ptr [g_nRLEEncodedMinimumX_004902e1]
        mov dword ptr [ebp-18h], eax
        mov eax, dword ptr [g_nRLEEncodedMinimumY_004902e5]
        imul dword ptr [ebp-30h]
        add eax, dword ptr [ebp-2Ch]
        add eax, dword ptr [g_nRLEEncodedMinimumX_004902e1]
        mov esi, eax
        mov dword ptr [g_pRLEScanlineStart_004902c1], esi
        mov eax, dword ptr [g_nRLEEncodedMinimumY_004902e5]
        mov dword ptr [ebp-10h], eax
        jmp L404BF3
L404BD6:
        push dword ptr [ebp-1Ch]
        push dword ptr [ebp+0Ch]
        push dword ptr [ebp-18h]
        call EncodeRLEScanline
        add esp, 0Ch
        mov eax, dword ptr [ebp-30h]
        add dword ptr [g_pRLEScanlineStart_004902c1], eax
        inc dword ptr [ebp-10h]
L404BF3:
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [g_nRLEEncodedMaximumY_004902ed]
        jle L404BD6
        mov eax, dword ptr [g_pRLEOutputCursor_004902c9]
        sub eax, dword ptr [ebp+18h]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
EncodeRasterClipToRLEFrame endp

; Function start: 0x00404C0C
; unsigned int TranslateRLEFramePalette(unsigned char *shape, int frame);
; Remaps the colours of an RLE frame in place.
TranslateRLEFramePalette proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+0Ch]
        shl esi, 3
        add esi, 8
        add esi, dword ptr [ebp+8]
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi+0Ch]
        mov eax, dword ptr [esi+14h]
        inc eax
        sub eax, ebx
        mov ebx, eax
        jle L404C96
        add esi, 18h
L404C37:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L404C55
        jne L404C73
        jae L404C93
L404C42:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L404C55
        jne L404C73
        jb L404C42
        jae L404C93
L404C55:
        movzx ecx, al
        mov eax, 0
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [esi], al
        inc esi
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L404C55
        jae L404C93
        je L404C42
L404C73:
        movzx ecx, al
        mov eax, 0
L404C7B:
        mov al, byte ptr [esi]
        mov al, byte ptr [eax+g_abRasterPaletteTranslation_00491b08]
        mov byte ptr [esi], al
        inc esi
        loop L404C7B
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja L404C55
        jne L404C73
        jb L404C42
L404C93:
        dec ebx
        jne L404C37
L404C96:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
TranslateRLEFramePalette endp

; Function start: 0x00404C9E
; void EncodeRLEScanline(int pixelCount, unsigned char transparentColour, int
;                        sourceX);
; Classifies one scanline into RLE runs for EncodeRasterClipToRLEFrame.
EncodeRLEScanline proc
        push ebp
        mov ebp, esp
        add esp, -4
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [g_pRLEScanlineStart_004902c1]
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 0
        push 0
        call EmitRLEScanlineRun
        add esp, 0Ch
        mov dword ptr [ebp-4], 5
        mov ecx, dword ptr [ebp+8]
        or ecx, ecx
        je L404DFC
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        mov ah, al
        cmp ah, byte ptr [ebp+0Ch]
        je L404DCD
L404CE7:
        mov dword ptr [ebp-4], 1
        or ecx, ecx
        je L404DFC
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        xor ah, al
        or al, al
        je L404D9A
        cmp ah, byte ptr [ebp+0Ch]
        jne L404D25
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 1
        push 1
        call EmitRLEScanlineRun
        add esp, 0Ch
        jmp L404DCD
L404D25:
        or ecx, ecx
        je L404DFC
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        xor ah, al
        cmp ah, byte ptr [ebp+0Ch]
        jne L404D51
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 1
        push 1
        call EmitRLEScanlineRun
        add esp, 0Ch
        jmp L404DCD
L404D51:
        or al, al
        jne L404D25
        or ecx, ecx
        je L404DFC
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        xor ah, al
        cmp ah, byte ptr [ebp+0Ch]
        jne L404D81
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 1
        push 1
        call EmitRLEScanlineRun
        add esp, 0Ch
        jmp L404DCD
L404D81:
        or al, al
        jne L404D25
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 3
        push 1
        call EmitRLEScanlineRun
        add esp, 0Ch
L404D9A:
        mov dword ptr [ebp-4], 2
        or ecx, ecx
        je L404DFC
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        je L404D9A
        xor ah, al
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 1
        push 2
        call EmitRLEScanlineRun
        add esp, 0Ch
        cmp ah, byte ptr [ebp+0Ch]
        jne L404CE7
L404DCD:
        mov dword ptr [ebp-4], 3
        or ecx, ecx
        je L404DFC
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        je L404DCD
        xor ah, al
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 1
        push 3
        call EmitRLEScanlineRun
        add esp, 0Ch
        jmp L404CE7
L404DFC:
        mov dword ptr [g_pRLEScanCursor_004902c5], esi
        push dword ptr [ebp+10h]
        push 0
        push dword ptr [ebp-4]
        call EmitRLEScanlineRun
        add esp, 0Ch
        push dword ptr [ebp+10h]
        push 0
        push 4
        call EmitRLEScanlineRun
        add esp, 0Ch
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
EncodeRLEScanline endp

; Function start: 0x00404E27
; void EmitRLEScanlineRun(int runType, int trailingCount, int sourceX);
; Emits one RLE run for EncodeRLEScanline.
EmitRLEScanlineRun proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        push eax
        push ecx
        mov esi, dword ptr [g_pRLELiteralStart_004902cd]
        mov edi, dword ptr [g_pRLEOutputCursor_004902c9]
        mov eax, dword ptr [ebp+8]
        cmp eax, 2
        je L404E83
        cmp eax, 1
        je L404F2A
        cmp eax, 3
        je L404FCF
        cmp eax, 4
        je L404FE2
        cmp eax, 0
        jne L404FF0
        xor eax, eax
        mov dword ptr [g_nRLEPendingSkip_004902bd], eax
        mov esi, dword ptr [g_pRLEScanCursor_004902c5]
        mov dword ptr [g_pRLELiteralStart_004902cd], esi
        jmp L404FF0
L404E83:
        mov ebx, dword ptr [g_nRLEPendingSkip_004902bd]
        or ebx, ebx
        je L404EC2
L404E8D:
        mov ecx, ebx
        cmp ecx, 0FFh
        jl L404E9C
        mov ecx, 0FFh
L404E9C:
        sub ebx, ecx
        cmp dword ptr [g_pRLEEncodeBuffer_004902b9], 0
        je L404EB3
        mov al, 1
        mov byte ptr [edi], al
        inc edi
        mov al, cl
        mov byte ptr [edi], al
        inc edi
        jmp L404EB6
L404EB3:
        add edi, 2
L404EB6:
        add esi, ecx
        or ebx, ebx
        jne L404E8D
        mov dword ptr [g_nRLEPendingSkip_004902bd], ebx
L404EC2:
        mov ebx, dword ptr [g_pRLEScanCursor_004902c5]
        sub ebx, esi
        sub ebx, dword ptr [ebp+0Ch]
        mov eax, dword ptr [ebp+10h]
        add eax, esi
        sub eax, dword ptr [g_pRLEScanlineStart_004902c1]
        cmp eax, dword ptr [g_nRLEEncodedMinimumX_004902e1]
        jge L404EE5
        mov dword ptr [g_nRLEEncodedMinimumX_004902e1], eax
L404EE5:
        add eax, ebx
        dec eax
        cmp eax, dword ptr [g_nRLEEncodedMaximumX_004902e9]
        jle L404EF7
        mov dword ptr [g_nRLEEncodedMaximumX_004902e9], eax
        jmp L404F21
L404EF7:
        mov ecx, ebx
        cmp ecx, 7Fh
        jl L404F03
        mov ecx, 7Fh
L404F03:
        cmp dword ptr [g_pRLEEncodeBuffer_004902b9], 0
        je L404F1A
        mov al, cl
        add al, al
        mov byte ptr [edi], al
        inc edi
        mov al, byte ptr [esi]
        mov byte ptr [edi], al
        inc edi
        jmp L404F1D
L404F1A:
        add edi, 2
L404F1D:
        add esi, ecx
        sub ebx, ecx
L404F21:
        or ebx, ebx
        jne L404EF7
        jmp L404FF0
L404F2A:
        mov ebx, dword ptr [g_nRLEPendingSkip_004902bd]
        or ebx, ebx
        je L404F69
L404F34:
        mov ecx, ebx
        cmp ecx, 0FFh
        jl L404F43
        mov ecx, 0FFh
L404F43:
        sub ebx, ecx
        cmp dword ptr [g_pRLEEncodeBuffer_004902b9], 0
        je L404F5A
        mov al, 1
        mov byte ptr [edi], al
        inc edi
        mov al, cl
        mov byte ptr [edi], al
        inc edi
        jmp L404F5D
L404F5A:
        add edi, 2
L404F5D:
        add esi, ecx
        or ebx, ebx
        jne L404F34
        mov dword ptr [g_nRLEPendingSkip_004902bd], ebx
L404F69:
        mov ebx, dword ptr [g_pRLEScanCursor_004902c5]
        sub ebx, esi
        sub ebx, dword ptr [ebp+0Ch]
        mov eax, dword ptr [ebp+10h]
        add eax, esi
        sub eax, dword ptr [g_pRLEScanlineStart_004902c1]
        cmp eax, dword ptr [g_nRLEEncodedMinimumX_004902e1]
        jge L404F8C
        mov dword ptr [g_nRLEEncodedMinimumX_004902e1], eax
L404F8C:
        add eax, ebx
        dec eax
        cmp eax, dword ptr [g_nRLEEncodedMaximumX_004902e9]
        jle L404F9E
        mov dword ptr [g_nRLEEncodedMaximumX_004902e9], eax
        jmp L404FC9
L404F9E:
        mov ecx, ebx
        cmp ecx, 7Fh
        jl L404FAA
        mov ecx, 7Fh
L404FAA:
        mov edx, ecx
        mov al, cl
        add al, al
        inc al
        cmp dword ptr [g_pRLEEncodeBuffer_004902b9], 0
        je L404FC2
        mov byte ptr [edi], al
        inc edi
        rep movsb
        jmp L404FC7
L404FC2:
        inc edi
        add esi, ecx
        add edi, ecx
L404FC7:
        sub ebx, edx
L404FC9:
        or ebx, ebx
        jne L404F9E
        jmp L404FF0
L404FCF:
        mov ebx, dword ptr [g_pRLEScanCursor_004902c5]
        sub ebx, esi
        sub ebx, dword ptr [ebp+0Ch]
        mov dword ptr [g_nRLEPendingSkip_004902bd], ebx
        jmp L404FF0
L404FE2:
        xor eax, eax
        cmp dword ptr [g_pRLEEncodeBuffer_004902b9], 0
        je L404FEF
        mov byte ptr [edi], al
L404FEF:
        inc edi
L404FF0:
        mov dword ptr [g_pRLEOutputCursor_004902c9], edi
        mov dword ptr [g_pRLELiteralStart_004902cd], esi
        pop ecx
        pop eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
EmitRLEScanlineRun endp

; Function start: 0x00405004
; int FillRasterClip(RasterClip *clip, int colour);
; Fills a clip with one colour.
FillRasterClip proc
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-18h], eax
        jle L405083
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L405083
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-1Ch], eax
        cmp eax, 0
        jg L405037
        mov eax, 0
L405037:
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-20h], eax
        cmp eax, 0
        jg L40504A
        mov eax, 0
L40504A:
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-18h]
        dec edx
        cmp eax, edx
        jl L40505A
        mov eax, edx
L40505A:
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L405069
        mov eax, edx
L405069:
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [ebp-0Ch]
        cmp eax, dword ptr [ebp-4]
        jl L40508E
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [ebp-8]
        jl L40508E
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-14h], eax
        jmp L405099
L405083:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L40508E:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405099:
        mov eax, dword ptr [ebp-8]
        imul dword ptr [ebp-18h]
        add eax, dword ptr [ebp-14h]
        add eax, dword ptr [ebp-4]
        mov edi, eax
        mov ebx, dword ptr [ebp-0Ch]
        inc ebx
        sub ebx, dword ptr [ebp-4]
        mov esi, dword ptr [ebp-18h]
        sub esi, ebx
        mov al, byte ptr [ebp+0Ch]
        mov ah, al
        shl eax, 10h
        mov al, byte ptr [ebp+0Ch]
        mov ah, al
        mov edx, dword ptr [ebp-8]
        jmp L4050D6
L4050C5:
        mov ecx, ebx
        and ecx, 3
        rep stosb
        mov ecx, ebx
        shr ecx, 2
        rep stosd
        add edi, esi
        inc edx
L4050D6:
        cmp edx, dword ptr [ebp-10h]
        jle L4050C5
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
FillRasterClip endp

; Function start: 0x004050E3
; int BlitRasterClip(RasterClip *source, int sourceX, int sourceY, RasterClip
;                    *destination, int destinationX, int destinationY,
;                    unsigned int colour);
; Copies one clip into another.
BlitRasterClip proc
        push ebp
        mov ebp, esp
        add esp, 0FFFFFF64h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-70h], eax
        jle L405165
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L405165
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-78h], eax
        cmp eax, 0
        jg L405119
        mov eax, 0
L405119:
        mov dword ptr [ebp-60h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-7Ch], eax
        cmp eax, 0
        jg L40512C
        mov eax, 0
L40512C:
        mov dword ptr [ebp-64h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-70h]
        dec edx
        cmp eax, edx
        jl L40513C
        mov eax, edx
L40513C:
        mov dword ptr [ebp-68h], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L40514B
        mov eax, edx
L40514B:
        mov dword ptr [ebp-6Ch], eax
        mov eax, dword ptr [ebp-68h]
        cmp eax, dword ptr [ebp-60h]
        jl L405170
        mov eax, dword ptr [ebp-6Ch]
        cmp eax, dword ptr [ebp-64h]
        jl L405170
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-74h], eax
        jmp L40517B
L405165:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405170:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L40517B:
        mov eax, dword ptr [ebp-60h]
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [ebp-64h]
        mov dword ptr [ebp-30h], eax
        mov eax, dword ptr [ebp-68h]
        mov dword ptr [ebp-34h], eax
        mov eax, dword ptr [ebp-6Ch]
        mov dword ptr [ebp-38h], eax
        mov eax, dword ptr [ebp-78h]
        sub dword ptr [ebp-2Ch], eax
        sub dword ptr [ebp-34h], eax
        mov eax, dword ptr [ebp-7Ch]
        sub dword ptr [ebp-30h], eax
        sub dword ptr [ebp-38h], eax
        mov esi, dword ptr [ebp+14h]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-94h], eax
        jle L40523C
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L40523C
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-98h], eax
        cmp eax, 0
        jg L4051D5
        mov eax, 0
L4051D5:
        mov dword ptr [ebp-80h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-9Ch], eax
        cmp eax, 0
        jg L4051EB
        mov eax, 0
L4051EB:
        mov dword ptr [ebp-84h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-94h]
        dec edx
        cmp eax, edx
        jl L405201
        mov eax, edx
L405201:
        mov dword ptr [ebp-88h], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L405213
        mov eax, edx
L405213:
        mov dword ptr [ebp-8Ch], eax
        mov eax, dword ptr [ebp-88h]
        cmp eax, dword ptr [ebp-80h]
        jl L405247
        mov eax, dword ptr [ebp-8Ch]
        cmp eax, dword ptr [ebp-84h]
        jl L405247
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-90h], eax
        jmp L405252
L40523C:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405247:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405252:
        mov eax, dword ptr [ebp-80h]
        mov dword ptr [ebp-40h], eax
        mov eax, dword ptr [ebp-84h]
        mov dword ptr [ebp-44h], eax
        mov eax, dword ptr [ebp-88h]
        mov dword ptr [ebp-48h], eax
        mov eax, dword ptr [ebp-8Ch]
        mov dword ptr [ebp-4Ch], eax
        mov eax, dword ptr [ebp-98h]
        sub dword ptr [ebp-40h], eax
        sub dword ptr [ebp-48h], eax
        mov eax, dword ptr [ebp-9Ch]
        sub dword ptr [ebp-44h], eax
        sub dword ptr [ebp-4Ch], eax
        mov eax, dword ptr [ebp+0Ch]
        sub eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-24h], eax
        mov eax, dword ptr [ebp+10h]
        sub eax, dword ptr [ebp+1Ch]
        mov dword ptr [ebp-28h], eax
        mov eax, dword ptr [ebp-2Ch]
        mov edx, dword ptr [ebp-40h]
        add edx, dword ptr [ebp-24h]
        cmp eax, edx
        jg L4052AC
        mov eax, edx
L4052AC:
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [ebp-30h]
        mov edx, dword ptr [ebp-44h]
        add edx, dword ptr [ebp-28h]
        cmp eax, edx
        jg L4052BE
        mov eax, edx
L4052BE:
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [ebp-34h]
        mov edx, dword ptr [ebp-48h]
        add edx, dword ptr [ebp-24h]
        cmp eax, edx
        jl L4052D0
        mov eax, edx
L4052D0:
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [ebp-38h]
        mov edx, dword ptr [ebp-4Ch]
        add edx, dword ptr [ebp-28h]
        cmp eax, edx
        jl L4052E2
        mov eax, edx
L4052E2:
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [ebp-0Ch]
        cmp eax, dword ptr [ebp-4]
        jl L405474
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [ebp-8]
        jl L405474
        mov eax, dword ptr [ebp-40h]
        mov edx, dword ptr [ebp-2Ch]
        sub edx, dword ptr [ebp-24h]
        cmp eax, edx
        jg L40530C
        mov eax, edx
L40530C:
        mov dword ptr [ebp-14h], eax
        mov eax, dword ptr [ebp-44h]
        mov edx, dword ptr [ebp-30h]
        sub edx, dword ptr [ebp-28h]
        cmp eax, edx
        jg L40531E
        mov eax, edx
L40531E:
        mov dword ptr [ebp-18h], eax
        mov eax, dword ptr [ebp-48h]
        mov edx, dword ptr [ebp-34h]
        sub edx, dword ptr [ebp-24h]
        cmp eax, edx
        jl L405330
        mov eax, edx
L405330:
        mov dword ptr [ebp-1Ch], eax
        mov eax, dword ptr [ebp-4Ch]
        mov edx, dword ptr [ebp-38h]
        sub edx, dword ptr [ebp-28h]
        cmp eax, edx
        jl L405342
        mov eax, edx
L405342:
        mov dword ptr [ebp-20h], eax
        mov eax, dword ptr [ebp-0Ch]
        inc eax
        sub eax, dword ptr [ebp-4]
        mov dword ptr [ebp-58h], eax
        mov eax, dword ptr [ebp-10h]
        inc eax
        sub eax, dword ptr [ebp-8]
        mov dword ptr [ebp-54h], eax
        mov eax, dword ptr [ebp-7Ch]
        imul dword ptr [ebp-70h]
        add eax, dword ptr [ebp-74h]
        add eax, dword ptr [ebp-78h]
        mov esi, eax
        mov eax, dword ptr [ebp-9Ch]
        imul dword ptr [ebp-94h]
        add eax, dword ptr [ebp-90h]
        add eax, dword ptr [ebp-98h]
        mov edi, eax
        mov eax, dword ptr [ebp-8]
        mov ebx, dword ptr [ebp-18h]
        cmp eax, ebx
        jle L4053AB
        mul dword ptr [ebp-70h]
        add esi, eax
        mov eax, ebx
        mul dword ptr [ebp-94h]
        add edi, eax
        mov eax, dword ptr [ebp-70h]
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [ebp-94h]
        mov dword ptr [ebp-50h], eax
        jmp L4053D1
L4053AB:
        mov eax, dword ptr [ebp-10h]
        mul dword ptr [ebp-70h]
        add esi, eax
        mov eax, dword ptr [ebp-20h]
        mul dword ptr [ebp-94h]
        add edi, eax
        mov eax, dword ptr [ebp-70h]
        neg eax
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [ebp-94h]
        neg eax
        mov dword ptr [ebp-50h], eax
L4053D1:
        mov ecx, dword ptr [ebp-58h]
        mov eax, dword ptr [ebp-4]
        mov ebx, dword ptr [ebp-14h]
        cmp eax, ebx
        jle L4053F2
        add esi, eax
        add edi, ebx
        sub dword ptr [ebp-3Ch], ecx
        sub dword ptr [ebp-50h], ecx
        cld
        mov dword ptr [ebp-5Ch], 0
        jmp L405406
L4053F2:
        add esi, dword ptr [ebp-0Ch]
        add edi, dword ptr [ebp-1Ch]
        add dword ptr [ebp-3Ch], ecx
        add dword ptr [ebp-50h], ecx
        std
        mov dword ptr [ebp-5Ch], 3
L405406:
        mov eax, dword ptr [ebp+20h]
        test eax, 0FFFFFF00h
        je L40543F
        mov edx, dword ptr [ebp-54h]
        mov eax, dword ptr [ebp-3Ch]
        mov ebx, dword ptr [ebp-50h]
L405419:
        mov ecx, dword ptr [ebp-58h]
        and ecx, 3
        rep movsb
        mov ecx, dword ptr [ebp-58h]
        shr ecx, 2
        sub esi, dword ptr [ebp-5Ch]
        sub edi, dword ptr [ebp-5Ch]
        rep movsd
        add esi, dword ptr [ebp-5Ch]
        add edi, dword ptr [ebp-5Ch]
        add esi, eax
        add edi, ebx
        dec edx
        jne L405419
        cld
        jmp L40546C
L40543F:
        mov dl, al
        mov ah, al
        shl eax, 10h
        mov al, dl
        mov ah, al
        mov edx, dword ptr [ebp-54h]
        mov ebx, dword ptr [ebp-50h]
L405450:
        mov ecx, dword ptr [ebp-58h]
        and ecx, 3
        rep stosb
        mov ecx, dword ptr [ebp-58h]
        shr ecx, 2
        sub edi, dword ptr [ebp-5Ch]
        rep stosd
        add edi, dword ptr [ebp-5Ch]
        add edi, ebx
        dec edx
        jne L405450
        cld
L40546C:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405474:
        mov eax, 0FFFFFFFDh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
BlitRasterClip endp

; Function start: 0x0040547F
; int ScrollRasterClipWrapped(RasterClip *clip, int deltaX, int deltaY, int
;                             mode, unsigned int fillOrScratch);
; Scrolls a clip, wrapping the rows that leave it.
ScrollRasterClipWrapped proc
        push ebp
        mov ebp, esp
        add esp, -44h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+0Ch]
        inc eax
        sub eax, dword ptr [esi+4]
        mov dword ptr [ebp-30h], eax
        jle L405671
        mov edx, dword ptr [esi+10h]
        inc edx
        sub edx, dword ptr [esi+8]
        mov dword ptr [ebp-34h], edx
        jle L405671
        neg eax
        mov dword ptr [ebp-38h], eax
        neg edx
        mov dword ptr [ebp-3Ch], edx
        cmp dword ptr [ebp+14h], 1
        je L405506
        mov eax, dword ptr [ebp+0Ch]
        cdq
        xor eax, edx
        sub eax, edx
        cmp eax, dword ptr [ebp-30h]
        jge L4054EA
        mov eax, dword ptr [ebp+10h]
        cdq
        xor eax, edx
        sub eax, edx
        cmp eax, dword ptr [ebp-34h]
        jge L4054EA
        mov eax, dword ptr [ebp+8]
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-44h], eax
        jmp L405571
L4054EA:
        mov eax, dword ptr [ebp+18h]
        push 0
        movzx ax, al
        push ax
        push dword ptr [ebp+8]
        call FillRasterClip
        add esp, 8
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405506:
        cmp dword ptr [ebp+18h], 0
        je L405665
        lea eax, dword ptr [ebp-28h]
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-14h], eax
        lea eax, dword ptr [ebp-14h]
        mov dword ptr [ebp-28h], eax
        xor eax, eax
        mov dword ptr [ebp-24h], eax
        mov dword ptr [ebp-20h], eax
        mov eax, dword ptr [ebp-30h]
        dec eax
        mov dword ptr [ebp-10h], eax
        mov dword ptr [ebp-1Ch], eax
        mov eax, dword ptr [ebp-34h]
        dec eax
        mov dword ptr [ebp-0Ch], eax
        mov dword ptr [ebp-18h], eax
        push -1
        push 0
        push 0
        push dword ptr [ebp-2Ch]
        push 0
        push 0
        push dword ptr [ebp+8]
        call BlitRasterClip
        add esp, 1Ch
        mov eax, dword ptr [ebp+0Ch]
        cdq
        idiv dword ptr [ebp-30h]
        mov dword ptr [ebp+0Ch], edx
        mov eax, dword ptr [ebp+10h]
        cdq
        idiv dword ptr [ebp-34h]
        mov dword ptr [ebp+10h], edx
        mov dword ptr [ebp-44h], 0FFFFFFFFh
L405571:
        mov eax, dword ptr [ebp+0Ch]
        or eax, dword ptr [ebp+10h]
        je L40565D
        mov esi, dword ptr [ebp-2Ch]
        mov edi, dword ptr [ebp+8]
        push -1
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push 0
        push 0
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push dword ptr [ebp-34h]
        push dword ptr [ebp-30h]
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push 0
        push dword ptr [ebp-30h]
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push dword ptr [ebp-3Ch]
        push dword ptr [ebp-30h]
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push dword ptr [ebp-34h]
        push 0
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push dword ptr [ebp-3Ch]
        push 0
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push dword ptr [ebp-34h]
        push dword ptr [ebp-38h]
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push 0
        push dword ptr [ebp-38h]
        push esi
        call BlitRasterClip
        add esp, 1Ch
        push dword ptr [ebp-44h]
        push dword ptr [ebp+10h]
        push dword ptr [ebp+0Ch]
        push edi
        push dword ptr [ebp-3Ch]
        push dword ptr [ebp-38h]
        push esi
        call BlitRasterClip
        add esp, 1Ch
L40565D:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405665:
        mov eax, dword ptr [ebp-30h]
        mul dword ptr [ebp-34h]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405671:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
ScrollRasterClipWrapped endp

; Function start: 0x0040567C
; unsigned int DrawRasterEllipse(RasterClip *clip, int x, int y, int
;                                horizontalRadius, int verticalRadius, int
;                                colour);
; Draws an ellipse outline.
DrawRasterEllipse proc
        push ebp
        mov ebp, esp
        add esp, -54h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        cmp dword ptr [ebp+14h], 0
        je L405695
        cmp dword ptr [ebp+18h], 0
        jne L4056C6
L405695:
        mov eax, dword ptr [ebp+10h]
        add eax, dword ptr [ebp+18h]
        mov ebx, dword ptr [ebp+0Ch]
        add ebx, dword ptr [ebp+14h]
        mov ecx, dword ptr [ebp+10h]
        sub ecx, dword ptr [ebp+18h]
        mov edx, dword ptr [ebp+0Ch]
        sub edx, dword ptr [ebp+14h]
        push dword ptr [ebp+1Ch]
        push 0
        push eax
        push ebx
        push ecx
        push edx
        push dword ptr [ebp+8]
        call DrawClippedLine
        add esp, 1Ch
        jmp L4059B7
L4056C6:
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-4Ch], eax
        jle L405738
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L405738
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-50h], eax
        cmp eax, 0
        jg L4056EC
        mov eax, 0
L4056EC:
        mov dword ptr [ebp-38h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-54h], eax
        cmp eax, 0
        jg L4056FF
        mov eax, 0
L4056FF:
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-4Ch]
        dec edx
        cmp eax, edx
        jl L40570F
        mov eax, edx
L40570F:
        mov dword ptr [ebp-40h], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L40571E
        mov eax, edx
L40571E:
        mov dword ptr [ebp-44h], eax
        mov eax, dword ptr [ebp-40h]
        cmp eax, dword ptr [ebp-38h]
        jl L405743
        mov eax, dword ptr [ebp-44h]
        cmp eax, dword ptr [ebp-3Ch]
        jl L405743
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-48h], eax
        jmp L40574E
L405738:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405743:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L40574E:
        mov eax, dword ptr [ebp+1Ch]
        mov ah, al
        mov dword ptr [ebp+1Ch], eax
        mov word ptr [ebp+1Eh], ax
        mov eax, dword ptr [ebp-50h]
        add dword ptr [ebp+0Ch], eax
        mov eax, dword ptr [ebp-54h]
        add dword ptr [ebp+10h], eax
        mov eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-8], eax
        mov dword ptr [ebp-0Ch], 0
        mov eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-10h], eax
        mul eax
        mov dword ptr [ebp-1Ch], eax
        shl eax, 1
        mov dword ptr [ebp-20h], eax
        mov eax, dword ptr [ebp+14h]
        mul eax
        mov dword ptr [ebp-14h], eax
        shl eax, 1
        mov dword ptr [ebp-18h], eax
        mov dword ptr [ebp-24h], 0
        mov eax, dword ptr [ebp-18h]
        mul dword ptr [ebp+18h]
        mov dword ptr [ebp-28h], eax
        mov eax, dword ptr [ebp-14h]
        shr eax, 2
        add eax, dword ptr [ebp-1Ch]
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [ebp-14h]
        mul dword ptr [ebp+18h]
        sub dword ptr [ebp-2Ch], eax
        mov ebx, dword ptr [ebp+18h]
L4057BE:
        mov eax, dword ptr [ebp-24h]
        sub eax, dword ptr [ebp-28h]
        jns L4058B4
        push ebx
        mov ecx, dword ptr [ebp+1Ch]
        mov edi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        add edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L4057FC
        cmp edi, dword ptr [ebp-40h]
        jg L4057FC
        cmp edx, dword ptr [ebp-3Ch]
        jl L4057FC
        cmp edx, dword ptr [ebp-44h]
        jg L4057FC
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L4057FC:
        mov edi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        sub edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L40582A
        cmp edi, dword ptr [ebp-40h]
        jg L40582A
        cmp edx, dword ptr [ebp-3Ch]
        jl L40582A
        cmp edx, dword ptr [ebp-44h]
        jg L40582A
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L40582A:
        mov edi, dword ptr [ebp-4]
        sub edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        add edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L405858
        cmp edi, dword ptr [ebp-40h]
        jg L405858
        cmp edx, dword ptr [ebp-3Ch]
        jl L405858
        cmp edx, dword ptr [ebp-44h]
        jg L405858
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L405858:
        mov edi, dword ptr [ebp-4]
        sub edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        sub edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L405886
        cmp edi, dword ptr [ebp-40h]
        jg L405886
        cmp edx, dword ptr [ebp-3Ch]
        jl L405886
        cmp edx, dword ptr [ebp-44h]
        jg L405886
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L405886:
        pop ebx
        cmp dword ptr [ebp-2Ch], 0
        js L40589D
        dec dword ptr [ebp-10h]
        dec ebx
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [ebp-18h]
        mov dword ptr [ebp-28h], eax
        sub dword ptr [ebp-2Ch], eax
L40589D:
        inc dword ptr [ebp-0Ch]
        mov eax, dword ptr [ebp-24h]
        add eax, dword ptr [ebp-20h]
        mov dword ptr [ebp-24h], eax
        add eax, dword ptr [ebp-1Ch]
        add dword ptr [ebp-2Ch], eax
        jmp L4057BE
L4058B4:
        mov eax, dword ptr [ebp-14h]
        sub eax, dword ptr [ebp-1Ch]
        mov edx, eax
        sar eax, 1
        add eax, edx
        sub eax, dword ptr [ebp-24h]
        sub eax, dword ptr [ebp-28h]
        sar eax, 1
        add dword ptr [ebp-2Ch], eax
L4058CB:
        push ebx
        mov ecx, dword ptr [ebp+1Ch]
        mov edi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        add edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L4058FD
        cmp edi, dword ptr [ebp-40h]
        jg L4058FD
        cmp edx, dword ptr [ebp-3Ch]
        jl L4058FD
        cmp edx, dword ptr [ebp-44h]
        jg L4058FD
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L4058FD:
        mov edi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        sub edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L40592B
        cmp edi, dword ptr [ebp-40h]
        jg L40592B
        cmp edx, dword ptr [ebp-3Ch]
        jl L40592B
        cmp edx, dword ptr [ebp-44h]
        jg L40592B
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L40592B:
        mov edi, dword ptr [ebp-4]
        sub edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        add edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L405959
        cmp edi, dword ptr [ebp-40h]
        jg L405959
        cmp edx, dword ptr [ebp-3Ch]
        jl L405959
        cmp edx, dword ptr [ebp-44h]
        jg L405959
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L405959:
        mov edi, dword ptr [ebp-4]
        sub edi, dword ptr [ebp-0Ch]
        mov edx, dword ptr [ebp-8]
        sub edx, dword ptr [ebp-10h]
        cmp edi, dword ptr [ebp-38h]
        jl L405987
        cmp edi, dword ptr [ebp-40h]
        jg L405987
        cmp edx, dword ptr [ebp-3Ch]
        jl L405987
        cmp edx, dword ptr [ebp-44h]
        jg L405987
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov byte ptr [edi], cl
L405987:
        pop ebx
        cmp dword ptr [ebp-2Ch], 0
        jns L40599D
        inc dword ptr [ebp-0Ch]
        mov eax, dword ptr [ebp-24h]
        add eax, dword ptr [ebp-20h]
        mov dword ptr [ebp-24h], eax
        add dword ptr [ebp-2Ch], eax
L40599D:
        dec dword ptr [ebp-10h]
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [ebp-18h]
        mov dword ptr [ebp-28h], eax
        sub eax, dword ptr [ebp-14h]
        sub dword ptr [ebp-2Ch], eax
        dec ebx
        js L4059B7
        jmp L4058CB
L4059B7:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DrawRasterEllipse endp

; Function start: 0x004059BD
; unsigned int FillRasterEllipse(RasterClip *clip, int x, int y, int
;                                horizontalRadius, int verticalRadius, int
;                                colour);
; Fills an ellipse.
FillRasterEllipse proc
        push ebp
        mov ebp, esp
        add esp, -54h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        cmp dword ptr [ebp+14h], 0
        je L4059D6
        cmp dword ptr [ebp+18h], 0
        jne L405A07
L4059D6:
        mov eax, dword ptr [ebp+10h]
        add eax, dword ptr [ebp+18h]
        mov ebx, dword ptr [ebp+0Ch]
        add ebx, dword ptr [ebp+14h]
        mov ecx, dword ptr [ebp+10h]
        sub ecx, dword ptr [ebp+18h]
        mov edx, dword ptr [ebp+0Ch]
        sub edx, dword ptr [ebp+14h]
        push dword ptr [ebp+1Ch]
        push 0
        push eax
        push ebx
        push ecx
        push edx
        push dword ptr [ebp+8]
        call DrawClippedLine
        add esp, 1Ch
        jmp L405CB5
L405A07:
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-4Ch], eax
        jle L405A79
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L405A79
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-50h], eax
        cmp eax, 0
        jg L405A2D
        mov eax, 0
L405A2D:
        mov dword ptr [ebp-38h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-54h], eax
        cmp eax, 0
        jg L405A40
        mov eax, 0
L405A40:
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-4Ch]
        dec edx
        cmp eax, edx
        jl L405A50
        mov eax, edx
L405A50:
        mov dword ptr [ebp-40h], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L405A5F
        mov eax, edx
L405A5F:
        mov dword ptr [ebp-44h], eax
        mov eax, dword ptr [ebp-40h]
        cmp eax, dword ptr [ebp-38h]
        jl L405A84
        mov eax, dword ptr [ebp-44h]
        cmp eax, dword ptr [ebp-3Ch]
        jl L405A84
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-48h], eax
        jmp L405A8F
L405A79:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405A84:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L405A8F:
        mov eax, dword ptr [ebp+1Ch]
        mov ah, al
        mov dword ptr [ebp+1Ch], eax
        mov word ptr [ebp+1Eh], ax
        mov eax, dword ptr [ebp-50h]
        add dword ptr [ebp+0Ch], eax
        mov eax, dword ptr [ebp-54h]
        add dword ptr [ebp+10h], eax
        mov eax, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [ebp+10h]
        mov dword ptr [ebp-8], eax
        mov dword ptr [ebp-0Ch], 0
        mov eax, dword ptr [ebp+18h]
        mov dword ptr [ebp-10h], eax
        mul eax
        mov dword ptr [ebp-1Ch], eax
        shl eax, 1
        mov dword ptr [ebp-20h], eax
        mov eax, dword ptr [ebp+14h]
        mul eax
        mov dword ptr [ebp-14h], eax
        shl eax, 1
        mov dword ptr [ebp-18h], eax
        mov dword ptr [ebp-24h], 0
        mov eax, dword ptr [ebp-18h]
        mul dword ptr [ebp+18h]
        mov dword ptr [ebp-28h], eax
        mov eax, dword ptr [ebp-14h]
        shr eax, 2
        add eax, dword ptr [ebp-1Ch]
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [ebp-14h]
        mul dword ptr [ebp+18h]
        sub dword ptr [ebp-2Ch], eax
        mov ebx, dword ptr [ebp+18h]
L405AFF:
        mov eax, dword ptr [ebp-24h]
        sub eax, dword ptr [ebp-28h]
        js L405B0C
        jmp L405BD4
L405B0C:
        mov edi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        cmp edi, dword ptr [ebp-38h]
        jl L405BA7
        cmp edi, dword ptr [ebp-40h]
        jl L405B23
        mov edi, dword ptr [ebp-40h]
L405B23:
        mov dword ptr [ebp-34h], edi
        mov edi, dword ptr [ebp-4]
        sub edi, dword ptr [ebp-0Ch]
        cmp edi, dword ptr [ebp-40h]
        jg L405BA7
        cmp edi, dword ptr [ebp-38h]
        jg L405B39
        mov edi, dword ptr [ebp-38h]
L405B39:
        mov dword ptr [ebp-30h], edi
        mov edx, dword ptr [ebp-8]
        add edx, dword ptr [ebp-10h]
        cmp edx, dword ptr [ebp-3Ch]
        jl L405BA7
        cmp edx, dword ptr [ebp-44h]
        jg L405B70
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp-34h]
        sub ecx, dword ptr [ebp-30h]
        inc ecx
        mov eax, dword ptr [ebp+1Ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
L405B70:
        mov edi, dword ptr [ebp-30h]
        mov edx, dword ptr [ebp-8]
        sub edx, dword ptr [ebp-10h]
        cmp edx, dword ptr [ebp-3Ch]
        jl L405BA7
        cmp edx, dword ptr [ebp-44h]
        jg L405BA7
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp-34h]
        sub ecx, dword ptr [ebp-30h]
        inc ecx
        mov eax, dword ptr [ebp+1Ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
L405BA7:
        cmp dword ptr [ebp-2Ch], 0
        js L405BBD
        dec dword ptr [ebp-10h]
        dec ebx
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [ebp-18h]
        mov dword ptr [ebp-28h], eax
        sub dword ptr [ebp-2Ch], eax
L405BBD:
        inc dword ptr [ebp-0Ch]
        mov eax, dword ptr [ebp-24h]
        add eax, dword ptr [ebp-20h]
        mov dword ptr [ebp-24h], eax
        add eax, dword ptr [ebp-1Ch]
        add dword ptr [ebp-2Ch], eax
        jmp L405AFF
L405BD4:
        mov eax, dword ptr [ebp-14h]
        sub eax, dword ptr [ebp-1Ch]
        mov edx, eax
        sar eax, 1
        add eax, edx
        sub eax, dword ptr [ebp-24h]
        sub eax, dword ptr [ebp-28h]
        sar eax, 1
        add dword ptr [ebp-2Ch], eax
L405BEB:
        mov edi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        cmp edi, dword ptr [ebp-38h]
        jl L405C86
        cmp edi, dword ptr [ebp-40h]
        jl L405C02
        mov edi, dword ptr [ebp-40h]
L405C02:
        mov dword ptr [ebp-34h], edi
        mov edi, dword ptr [ebp-4]
        sub edi, dword ptr [ebp-0Ch]
        cmp edi, dword ptr [ebp-40h]
        jg L405C86
        cmp edi, dword ptr [ebp-38h]
        jg L405C18
        mov edi, dword ptr [ebp-38h]
L405C18:
        mov dword ptr [ebp-30h], edi
        mov edx, dword ptr [ebp-8]
        add edx, dword ptr [ebp-10h]
        cmp edx, dword ptr [ebp-3Ch]
        jl L405C86
        cmp edx, dword ptr [ebp-44h]
        jg L405C4F
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp-34h]
        sub ecx, dword ptr [ebp-30h]
        inc ecx
        mov eax, dword ptr [ebp+1Ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
L405C4F:
        mov edi, dword ptr [ebp-30h]
        mov edx, dword ptr [ebp-8]
        sub edx, dword ptr [ebp-10h]
        cmp edx, dword ptr [ebp-3Ch]
        jl L405C86
        cmp edx, dword ptr [ebp-44h]
        jg L405C86
        mov eax, edx
        imul dword ptr [ebp-4Ch]
        add eax, dword ptr [ebp-48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp-34h]
        sub ecx, dword ptr [ebp-30h]
        inc ecx
        mov eax, dword ptr [ebp+1Ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
L405C86:
        cmp dword ptr [ebp-2Ch], 0
        jns L405C9B
        inc dword ptr [ebp-0Ch]
        mov eax, dword ptr [ebp-24h]
        add eax, dword ptr [ebp-20h]
        mov dword ptr [ebp-24h], eax
        add dword ptr [ebp-2Ch], eax
L405C9B:
        dec dword ptr [ebp-10h]
        mov eax, dword ptr [ebp-28h]
        sub eax, dword ptr [ebp-18h]
        mov dword ptr [ebp-28h], eax
        sub eax, dword ptr [ebp-14h]
        sub dword ptr [ebp-2Ch], eax
        dec ebx
        js L405CB5
        jmp L405BEB
L405CB5:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
FillRasterEllipse endp

; Quarter-wave cosine table, 901 entries, 16.16 fixed point.
g_anRLEQuarterCosine_00405cbb dd 10000h, 10000h, 10000h, 0FFFFh, 0FFFEh, 0FFFEh, 0FFFCh, 0FFFBh
                              dd 0FFFAh, 0FFF8h, 0FFF6h, 0FFF4h, 0FFF2h, 0FFEFh, 0FFECh, 0FFEAh
                              dd 0FFE6h, 0FFE3h, 0FFE0h, 0FFDCh, 0FFD8h, 0FFD4h, 0FFD0h, 0FFCBh
                              dd 0FFC7h, 0FFC2h, 0FFBDh, 0FFB7h, 0FFB2h, 0FFACh, 0FFA6h, 0FFA0h
                              dd 0FF9Ah, 0FF93h, 0FF8Dh, 0FF86h, 0FF7Fh, 0FF77h, 0FF70h, 0FF68h
                              dd 0FF60h, 0FF58h, 0FF50h, 0FF48h, 0FF3Fh, 0FF36h, 0FF2Dh, 0FF24h
                              dd 0FF1Ah, 0FF10h, 0FF07h, 0FEFDh, 0FEF2h, 0FEE8h, 0FEDDh, 0FED2h
                              dd 0FEC7h, 0FEBCh, 0FEB1h, 0FEA5h, 0FE99h, 0FE8Dh, 0FE81h, 0FE74h
                              dd 0FE68h, 0FE5Bh, 0FE4Eh, 0FE40h, 0FE33h, 0FE25h, 0FE18h, 0FE09h
                              dd 0FDFBh, 0FDEDh, 0FDDEh, 0FDCFh, 0FDC0h, 0FDB1h, 0FDA2h, 0FD92h
                              dd 0FD82h, 0FD72h, 0FD62h, 0FD52h, 0FD41h, 0FD30h, 0FD1Fh, 0FD0Eh
                              dd 0FCFDh, 0FCEBh, 0FCD9h, 0FCC7h, 0FCB5h, 0FCA3h, 0FC90h, 0FC7Dh
                              dd 0FC6Ah, 0FC57h, 0FC44h, 0FC30h, 0FC1Ch, 0FC08h, 0FBF4h, 0FBE0h
                              dd 0FBCBh, 0FBB7h, 0FBA2h, 0FB8Dh, 0FB77h, 0FB62h, 0FB4Ch, 0FB36h
                              dd 0FB20h, 0FB0Ah, 0FAF3h, 0FADCh, 0FAC5h, 0FAAEh, 0FA97h, 0FA80h
                              dd 0FA68h, 0FA50h, 0FA38h, 0FA20h, 0FA07h, 0F9EFh, 0F9D6h, 0F9BDh
                              dd 0F9A3h, 0F98Ah, 0F970h, 0F956h, 0F93Ch, 0F922h, 0F908h, 0F8EDh
                              dd 0F8D2h, 0F8B7h, 0F89Ch, 0F881h, 0F865h, 0F84Ah, 0F82Eh, 0F811h
                              dd 0F7F5h, 0F7D9h, 0F7BCh, 0F79Fh, 0F782h, 0F764h, 0F747h, 0F729h
                              dd 0F70Bh, 0F6EDh, 0F6CFh, 0F6B0h, 0F692h, 0F673h, 0F654h, 0F635h
                              dd 0F615h, 0F5F6h, 0F5D6h, 0F5B6h, 0F596h, 0F575h, 0F555h, 0F534h
                              dd 0F513h, 0F4F2h, 0F4D0h, 0F4AFh, 0F48Dh, 0F46Bh, 0F449h, 0F427h
                              dd 0F404h, 0F3E2h, 0F3BFh, 0F39Ch, 0F378h, 0F355h, 0F331h, 0F30Eh
                              dd 0F2EAh, 0F2C5h, 0F2A1h, 0F27Ch, 0F258h, 0F233h, 0F20Eh, 0F1E8h
                              dd 0F1C3h, 0F19Dh, 0F177h, 0F151h, 0F12Bh, 0F104h, 0F0DEh, 0F0B7h
                              dd 0F090h, 0F068h, 0F041h, 0F019h, 0EFF2h, 0EFCAh, 0EFA2h, 0EF79h
                              dd 0EF51h, 0EF28h, 0EEFFh, 0EED6h, 0EEADh, 0EE83h, 0EE5Ah, 0EE30h
                              dd 0EE06h, 0EDDCh, 0EDB1h, 0ED87h, 0ED5Ch, 0ED31h, 0ED06h, 0ECDBh
                              dd 0ECAFh, 0EC83h, 0EC58h, 0EC2Bh, 0EBFFh, 0EBD3h, 0EBA6h, 0EB79h
                              dd 0EB4Ch, 0EB1Fh, 0EAF2h, 0EAC4h, 0EA97h, 0EA69h, 0EA3Bh, 0EA0Dh
                              dd 0E9DEh, 0E9B0h, 0E981h, 0E952h, 0E923h, 0E8F3h, 0E8C4h, 0E894h
                              dd 0E864h, 0E834h, 0E804h, 0E7D3h, 0E7A3h, 0E772h, 0E741h, 0E710h
                              dd 0E6DEh, 0E6ADh, 0E67Bh, 0E649h, 0E617h, 0E5E5h, 0E5B3h, 0E580h
                              dd 0E54Dh, 0E51Ah, 0E4E7h, 0E4B4h, 0E481h, 0E44Dh, 0E419h, 0E3E5h
                              dd 0E3B1h, 0E37Ch, 0E348h, 0E313h, 0E2DEh, 0E2A9h, 0E274h, 0E23Eh
                              dd 0E209h, 0E1D3h, 0E19Dh, 0E167h, 0E131h, 0E0FAh, 0E0C3h, 0E08Dh
                              dd 0E056h, 0E01Eh, 0DFE7h, 0DFB0h, 0DF78h, 0DF40h, 0DF08h, 0DED0h
                              dd 0DE97h, 0DE5Fh, 0DE26h, 0DDEDh, 0DDB4h, 0DD7Bh, 0DD41h, 0DD07h
                              dd 0DCCEh, 0DC94h, 0DC5Ah, 0DC1Fh, 0DBE5h, 0DBAAh, 0DB6Fh, 0DB34h
                              dd 0DAF9h, 0DABEh, 0DA82h, 0DA47h, 0DA0Bh, 0D9CFh, 0D993h, 0D956h
                              dd 0D91Ah, 0D8DDh, 0D8A0h, 0D863h, 0D826h, 0D7E9h, 0D7ABh, 0D76Dh
                              dd 0D72Fh, 0D6F1h, 0D6B3h, 0D675h, 0D636h, 0D5F7h, 0D5B9h, 0D57Ah
                              dd 0D53Ah, 0D4FBh, 0D4BBh, 0D47Ch, 0D43Ch, 0D3FCh, 0D3BCh, 0D37Bh
                              dd 0D33Bh, 0D2FAh, 0D2B9h, 0D278h, 0D237h, 0D1F5h, 0D1B4h, 0D172h
                              dd 0D130h, 0D0EEh, 0D0ACh, 0D06Ah, 0D027h, 0CFE5h, 0CFA2h, 0CF5Fh
                              dd 0CF1Ch, 0CED8h, 0CE95h, 0CE51h, 0CE0Eh, 0CDCAh, 0CD85h, 0CD41h
                              dd 0CCFDh, 0CCB8h, 0CC73h, 0CC2Eh, 0CBE9h, 0CBA4h, 0CB5Fh, 0CB19h
                              dd 0CAD3h, 0CA8Eh, 0CA48h, 0CA01h, 0C9BBh, 0C975h, 0C92Eh, 0C8E7h
                              dd 0C8A0h, 0C859h, 0C812h, 0C7CAh, 0C783h, 0C73Bh, 0C6F3h, 0C6ABh
                              dd 0C663h, 0C61Ah, 0C5D2h, 0C589h, 0C540h, 0C4F7h, 0C4AEh, 0C465h
                              dd 0C41Bh, 0C3D2h, 0C388h, 0C33Eh, 0C2F4h, 0C2AAh, 0C260h, 0C215h
                              dd 0C1CAh, 0C180h, 0C135h, 0C0EAh, 0C09Eh, 0C053h, 0C007h, 0BFBCh
                              dd 0BF70h, 0BF24h, 0BED8h, 0BE8Bh, 0BE3Fh, 0BDF2h, 0BDA5h, 0BD58h
                              dd 0BD0Bh, 0BCBEh, 0BC71h, 0BC23h, 0BBD6h, 0BB88h, 0BB3Ah, 0BAECh
                              dd 0BA9Eh, 0BA4Fh, 0BA01h, 0B9B2h, 0B963h, 0B914h, 0B8C5h, 0B876h
                              dd 0B827h, 0B7D7h, 0B787h, 0B738h, 0B6E8h, 0B698h, 0B647h, 0B5F7h
                              dd 0B5A6h, 0B556h, 0B505h, 0B4B4h, 0B463h, 0B412h, 0B3C0h, 0B36Fh
                              dd 0B31Dh, 0B2CBh, 0B279h, 0B227h, 0B1D5h, 0B183h, 0B130h, 0B0DEh
                              dd 0B08Bh, 0B038h, 0AFE5h, 0AF92h, 0AF3Eh, 0AEEBh, 0AE97h, 0AE44h
                              dd 0ADF0h, 0AD9Ch, 0AD48h, 0ACF3h, 0AC9Fh, 0AC4Bh, 0ABF6h, 0ABA1h
                              dd 0AB4Ch, 0AAF7h, 0AAA2h, 0AA4Dh, 0A9F7h, 0A9A1h, 0A94Ch, 0A8F6h
                              dd 0A8A0h, 0A84Ah, 0A7F3h, 0A79Dh, 0A747h, 0A6F0h, 0A699h, 0A642h
                              dd 0A5EBh, 0A594h, 0A53Dh, 0A4E5h, 0A48Eh, 0A436h, 0A3DEh, 0A386h
                              dd 0A32Eh, 0A2D6h, 0A27Eh, 0A225h, 0A1CDh, 0A174h, 0A11Bh, 0A0C2h
                              dd 0A069h, 0A010h, 9FB7h, 9F5Dh, 9F04h, 9EAAh, 9E50h, 9DF6h
                              dd 9D9Ch, 9D42h, 9CE7h, 9C8Dh, 9C32h, 9BD8h, 9B7Dh, 9B22h
                              dd 9AC7h, 9A6Ch, 9A11h, 99B5h, 995Ah, 98FEh, 98A2h, 9846h
                              dd 97EAh, 978Eh, 9732h, 96D6h, 9679h, 961Ch, 95C0h, 9563h
                              dd 9506h, 94A9h, 944Ch, 93EEh, 9391h, 9334h, 92D6h, 9278h
                              dd 921Ah, 91BCh, 915Eh, 9100h, 90A2h, 9043h, 8FE5h, 8F86h
                              dd 8F27h, 8EC8h, 8E69h, 8E0Ah, 8DABh, 8D4Ch, 8CECh, 8C8Dh
                              dd 8C2Dh, 8BCDh, 8B6Dh, 8B0Dh, 8AADh, 8A4Dh, 89EDh, 898Ch
                              dd 892Ch, 88CBh, 886Bh, 880Ah, 87A9h, 8748h, 86E7h, 8685h
                              dd 8624h, 85C2h, 8561h, 84FFh, 849Dh, 843Ch, 83DAh, 8377h
                              dd 8315h, 82B3h, 8251h, 81EEh, 818Bh, 8129h, 80C6h, 8063h
                              dd 8000h, 7F9Dh, 7F3Ah, 7ED6h, 7E73h, 7E0Fh, 7DACh, 7D48h
                              dd 7CE4h, 7C80h, 7C1Ch, 7BB8h, 7B54h, 7AF0h, 7A8Ch, 7A27h
                              dd 79C3h, 795Eh, 78F9h, 7894h, 782Fh, 77CAh, 7765h, 7700h
                              dd 769Bh, 7635h, 75D0h, 756Ah, 7504h, 749Fh, 7439h, 73D3h
                              dd 736Dh, 7307h, 72A0h, 723Ah, 71D4h, 716Dh, 7107h, 70A0h
                              dd 7039h, 6FD2h, 6F6Bh, 6F04h, 6E9Dh, 6E36h, 6DCFh, 6D67h
                              dd 6D00h, 6C98h, 6C31h, 6BC9h, 6B61h, 6AF9h, 6A91h, 6A29h
                              dd 69C1h, 6959h, 68F1h, 6888h, 6820h, 67B7h, 674Fh, 66E6h
                              dd 667Dh, 6614h, 65ABh, 6542h, 64D9h, 6470h, 6407h, 639Eh
                              dd 6334h, 62CBh, 6261h, 61F8h, 618Eh, 6124h, 60BAh, 6050h
                              dd 5FE6h, 5F7Ch, 5F12h, 5EA8h, 5E3Dh, 5DD3h, 5D69h, 5CFEh
                              dd 5C93h, 5C29h, 5BBEh, 5B53h, 5AE8h, 5A7Dh, 5A12h, 59A7h
                              dd 593Ch, 58D1h, 5865h, 57FAh, 578Fh, 5723h, 56B8h, 564Ch
                              dd 55E0h, 5574h, 5509h, 549Dh, 5431h, 53C5h, 5358h, 52ECh
                              dd 5280h, 5214h, 51A7h, 513Bh, 50CEh, 5062h, 4FF5h, 4F88h
                              dd 4F1Ch, 4EAFh, 4E42h, 4DD5h, 4D68h, 4CFBh, 4C8Eh, 4C21h
                              dd 4BB4h, 4B46h, 4AD9h, 4A6Bh, 49FEh, 4990h, 4923h, 48B5h
                              dd 4848h, 47DAh, 476Ch, 46FEh, 4690h, 4622h, 45B4h, 4546h
                              dd 44D8h, 446Ah, 43FBh, 438Dh, 431Fh, 42B0h, 4242h, 41D3h
                              dd 4165h, 40F6h, 4088h, 4019h, 3FAAh, 3F3Bh, 3ECCh, 3E5Eh
                              dd 3DEFh, 3D80h, 3D11h, 3CA1h, 3C32h, 3BC3h, 3B54h, 3AE5h
                              dd 3A75h, 3A06h, 3996h, 3927h, 38B7h, 3848h, 37D8h, 3769h
                              dd 36F9h, 3689h, 3619h, 35AAh, 353Ah, 34CAh, 345Ah, 33EAh
                              dd 337Ah, 330Ah, 329Ah, 322Ah, 31B9h, 3149h, 30D9h, 3069h
                              dd 2FF8h, 2F88h, 2F17h, 2EA7h, 2E37h, 2DC6h, 2D55h, 2CE5h
                              dd 2C74h, 2C04h, 2B93h, 2B22h, 2AB1h, 2A41h, 29D0h, 295Fh
                              dd 28EEh, 287Dh, 280Ch, 279Bh, 272Ah, 26B9h, 2648h, 25D7h
                              dd 2566h, 24F5h, 2483h, 2412h, 23A1h, 2330h, 22BEh, 224Dh
                              dd 21DCh, 216Ah, 20F9h, 2087h, 2016h, 1FA4h, 1F33h, 1EC1h
                              dd 1E50h, 1DDEh, 1D6Dh, 1CFBh, 1C89h, 1C18h, 1BA6h, 1B34h
                              dd 1AC2h, 1A51h, 19DFh, 196Dh, 18FBh, 1889h, 1817h, 17A6h
                              dd 1734h, 16C2h, 1650h, 15DEh, 156Ch, 14FAh, 1488h, 1416h
                              dd 13A4h, 1332h, 12C0h, 124Eh, 11DCh, 1169h, 10F7h, 1085h
                              dd 1013h, 0FA1h, 0F2Fh, 0EBDh, 0E4Ah, 0DD8h, 0D66h, 0CF4h
                              dd 0C81h, 0C0Fh, 0B9Dh, 0B2Bh, 0AB8h, 0A46h, 9D4h, 961h
                              dd 8EFh, 87Dh, 80Bh, 798h, 726h, 6B4h, 641h, 5CFh
                              dd 55Ch, 4EAh, 478h, 405h, 393h, 321h, 2AEh, 23Ch
                              dd 1CAh, 157h, 0E5h, 72h, 0

; Function start: 0x00406ACF
; void GetRLETransformTrig(int angleTenths, int *cosine, int *sine);
; Looks up sine and cosine in the quarter-wave table.
GetRLETransformTrig proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov ebx, dword ptr [ebp+8]
        and ebx, ebx
        jns L406AED
L406ADD:
        add ebx, 0E10h
        js L406ADD
        jmp L406AED
L406AE7:
        sub ebx, 0E10h
L406AED:
        cmp ebx, 0E10h
        jg L406AE7
        cmp ebx, 708h
        ja L406B35
        cmp ebx, 384h
        ja L406B18
        shl ebx, 2
        mov eax, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb]
        neg ebx
        mov edx, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb+0E10h]
        jmp L406B77
L406B18:
        neg ebx
        add ebx, 708h
        shl ebx, 2
        mov eax, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb]
        neg eax
        neg ebx
        mov edx, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb+0E10h]
        jmp L406B77
L406B35:
        neg ebx
        add ebx, 0E10h
        cmp ebx, 384h
        ja L406B5A
        shl ebx, 2
        mov eax, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb]
        neg ebx
        mov edx, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb+0E10h]
        neg edx
        jmp L406B77
L406B5A:
        neg ebx
        add ebx, 708h
        shl ebx, 2
        mov eax, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb]
        neg eax
        neg ebx
        mov edx, dword ptr [ebx+g_anRLEQuarterCosine_00405cbb+0E10h]
        neg edx
L406B77:
        mov ebx, dword ptr [ebp+0Ch]
        mov dword ptr [ebx], eax
        mov ebx, dword ptr [ebp+10h]
        mov dword ptr [ebx], edx
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRLETransformTrig endp

; Function start: 0x00406B87
; void CalculateRoundedRLEFixedProduct(int left, int right, int *result);
; Multiplies two fixed-point values with rounding.
CalculateRoundedRLEFixedProduct proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov eax, dword ptr [ebp+8]
        imul dword ptr [ebp+0Ch]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov edi, dword ptr [ebp+10h]
        mov dword ptr [edi], eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CalculateRoundedRLEFixedProduct endp

; Function start: 0x00406BAD
; void TransformRLEPoint(int *point, int *result, int *origin, unsigned int
;                        angleTenths, int scaleX, int scaleY);
; Rotates and scales one point for RotateRLEImage.
TransformRLEPoint proc
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        lea eax, dword ptr [ebp-8]
        push eax
        lea eax, dword ptr [ebp-4]
        push eax
        push dword ptr [ebp+14h]
        call GetRLETransformTrig
        add esp, 0Ch
        mov esi, dword ptr [ebp+8]
        mov edi, dword ptr [ebp+10h]
        mov eax, dword ptr [esi]
        sub eax, dword ptr [edi]
        shl eax, 10h
        imul dword ptr [ebp+18h]
        add eax, 8000h
        adc edx, 0
        mov ebx, edx
        mov eax, ebx
        imul dword ptr [ebp-4]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp-0Ch], eax
        mov eax, ebx
        imul dword ptr [ebp-8]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp-14h], eax
        mov eax, dword ptr [esi+4]
        sub eax, dword ptr [edi+4]
        shl eax, 10h
        imul dword ptr [ebp+1Ch]
        add eax, 8000h
        adc edx, 0
        mov ecx, edx
        mov eax, ecx
        imul dword ptr [ebp-4]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp-18h], eax
        mov eax, ecx
        imul dword ptr [ebp-8]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp-10h], eax
        mov esi, dword ptr [ebp+0Ch]
        mov edx, dword ptr [ebp-0Ch]
        sub edx, dword ptr [ebp-10h]
        add edx, dword ptr [edi]
        mov dword ptr [esi], edx
        mov edx, dword ptr [ebp-18h]
        add edx, dword ptr [ebp-14h]
        add edx, dword ptr [edi+4]
        mov dword ptr [esi+4], edx
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
TransformRLEPoint endp

; Function start: 0x00406C74
; unsigned int GetRawImageHeight(unsigned char *shape);
; Returns the height of a raw image.
GetRawImageHeight proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+8]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRawImageHeight endp

; Function start: 0x00406C87
; unsigned int GetRawFrameWidth(unsigned char *shape, int frame);
; Returns the width of a raw image frame.
GetRawFrameWidth proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 2
        add eax, dword ptr [ebp+8]
        add eax, 10h
        mov esi, dword ptr [eax]
        add esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRawFrameWidth endp

; Function start: 0x00406CA7
; int BlitRawFrame(RasterClip *clip, int x, int y, unsigned char *shape, int
;                  frame, unsigned char *translation);
; Draws a raw image frame through a clip.
BlitRawFrame proc
        push ebp
        mov ebp, esp
        add esp, -30h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-28h], eax
        jle L406D26
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L406D26
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-2Ch], eax
        cmp eax, 0
        jg L406CDA
        mov eax, 0
L406CDA:
        mov dword ptr [ebp-14h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-30h], eax
        cmp eax, 0
        jg L406CED
        mov eax, 0
L406CED:
        mov dword ptr [ebp-18h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-28h]
        dec edx
        cmp eax, edx
        jl L406CFD
        mov eax, edx
L406CFD:
        mov dword ptr [ebp-1Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L406D0C
        mov eax, edx
L406D0C:
        mov dword ptr [ebp-20h], eax
        mov eax, dword ptr [ebp-1Ch]
        cmp eax, dword ptr [ebp-14h]
        jl L406D31
        mov eax, dword ptr [ebp-20h]
        cmp eax, dword ptr [ebp-18h]
        jl L406D31
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-24h], eax
        jmp L406D3C
L406D26:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L406D31:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L406D3C:
        mov eax, dword ptr [ebp-2Ch]
        add dword ptr [ebp+0Ch], eax
        mov eax, dword ptr [ebp-30h]
        add dword ptr [ebp+10h], eax
        mov esi, dword ptr [ebp+14h]
        mov edx, dword ptr [esi+8]
        mov eax, dword ptr [ebp+18h]
        shl eax, 2
        add eax, dword ptr [ebp+14h]
        add eax, 10h
        mov esi, dword ptr [eax]
        add esi, dword ptr [ebp+14h]
        mov dword ptr [ebp-8], 0
        mov ecx, dword ptr [esi]
        mov dword ptr [ebp-4], ecx
        cmp ecx, 0
        je L406E2E
        add esi, 4
        mov edi, dword ptr [ebp+8]
        mov eax, dword ptr [ebp-1Ch]
        inc eax
        sub eax, ecx
        sub eax, dword ptr [ebp+0Ch]
        jns L406D8D
        add ecx, eax
        jle L406E2E
L406D8D:
        mov eax, dword ptr [ebp+0Ch]
        sub eax, dword ptr [ebp-14h]
        jns L406DA2
        add ecx, eax
        jle L406E2E
        sub esi, eax
        sub dword ptr [ebp+0Ch], eax
L406DA2:
        mov eax, dword ptr [ebp-20h]
        inc eax
        sub eax, edx
        sub eax, dword ptr [ebp+10h]
        jns L406DB1
        add edx, eax
        jle L406E2E
L406DB1:
        mov eax, dword ptr [ebp+10h]
        sub eax, dword ptr [ebp-18h]
        jns L406DC6
        add edx, eax
        jle L406E2E
        sub dword ptr [ebp+10h], eax
        imul eax, dword ptr [ebp-4]
        sub esi, eax
L406DC6:
        mov dword ptr [ebp-10h], edx
        mov eax, dword ptr [ebp+10h]
        imul dword ptr [ebp-28h]
        add eax, dword ptr [ebp-24h]
        add eax, dword ptr [ebp+0Ch]
        mov edi, eax
        mov dword ptr [ebp-8], ecx
        sub dword ptr [ebp-4], ecx
        mov eax, dword ptr [ebp-28h]
        sub eax, ecx
        mov dword ptr [ebp-0Ch], eax
        mov edx, dword ptr [ebp-10h]
        cmp dword ptr [ebp+1Ch], 0
        jne L406E10
L406DEE:
        shr ecx, 1
        rep movsw
        adc ecx, 0
        rep movsb
        mov ecx, dword ptr [ebp-8]
        add esi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        dec edx
        jne L406DEE
        mov eax, dword ptr [ebp-4]
        add eax, dword ptr [ebp-8]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L406E10:
        jecxz L406E2E
        mov ebx, dword ptr [ebp+1Ch]
L406E15:
        mov al, byte ptr [esi]
        xlatb
        cmp al, 0FFh
        je L406E1E
        mov byte ptr [edi], al
L406E1E:
        inc esi
        inc edi
        loop L406E15
        mov ecx, dword ptr [ebp-8]
        add esi, dword ptr [ebp-4]
        add edi, dword ptr [ebp-0Ch]
        dec edx
        jne L406E15
L406E2E:
        mov eax, dword ptr [ebp-4]
        add eax, dword ptr [ebp-8]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
BlitRawFrame endp

; Function start: 0x00406E3A
; void BlitSelectedRawFrames(RasterClip *clip, int x, int y, unsigned char
;                            *shape, const unsigned char *frames, unsigned
;                            char *translation);
; Draws several raw image frames.
BlitSelectedRawFrames proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+18h]
        mov edi, dword ptr [ebp+0Ch]
L406E4A:
        movzx eax, byte ptr [esi]
        push dword ptr [ebp+1Ch]
        push eax
        push dword ptr [ebp+14h]
        push dword ptr [ebp+10h]
        push edi
        push dword ptr [ebp+8]
        call BlitRawFrame
        add esp, 18h
        add edi, eax
        inc esi
        cmp byte ptr [esi], 0
        jne L406E4A
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
BlitSelectedRawFrames endp

; Function start: 0x00406E71
; int BlitRawScanline(RasterClip *clip, int y, const unsigned char *pixels,
;                     int width);
; Draws one raw scanline.
BlitRawScanline proc
        push ebp
        mov ebp, esp
        add esp, -24h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-1Ch], eax
        jle L406EF0
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L406EF0
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-20h], eax
        cmp eax, 0
        jg L406EA4
        mov eax, 0
L406EA4:
        mov dword ptr [ebp-8], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-24h], eax
        cmp eax, 0
        jg L406EB7
        mov eax, 0
L406EB7:
        mov dword ptr [ebp-0Ch], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-1Ch]
        dec edx
        cmp eax, edx
        jl L406EC7
        mov eax, edx
L406EC7:
        mov dword ptr [ebp-10h], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L406ED6
        mov eax, edx
L406ED6:
        mov dword ptr [ebp-14h], eax
        mov eax, dword ptr [ebp-10h]
        cmp eax, dword ptr [ebp-8]
        jl L406EFB
        mov eax, dword ptr [ebp-14h]
        cmp eax, dword ptr [ebp-0Ch]
        jl L406EFB
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-18h], eax
        jmp L406F06
L406EF0:
        mov eax, 0FFFFFFFFh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L406EFB:
        mov eax, 0FFFFFFFEh
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
L406F06:
        mov dword ptr [ebp-4], 0
        mov eax, dword ptr [ebp-20h]
        add dword ptr [ebp-4], eax
        mov eax, dword ptr [ebp-24h]
        add dword ptr [ebp+0Ch], eax
        mov esi, dword ptr [ebp+10h]
        mov edi, dword ptr [ebp+8]
        mov ecx, dword ptr [ebp+14h]
        mov eax, dword ptr [ebp-10h]
        sub eax, dword ptr [ebp-4]
        inc eax
        sub eax, ecx
        jns L406F31
        add ecx, eax
        jle L406F6E
L406F31:
        mov eax, dword ptr [ebp-4]
        sub eax, dword ptr [ebp-8]
        jns L406F42
        add ecx, eax
        jle L406F6E
        sub esi, eax
        sub dword ptr [ebp-4], eax
L406F42:
        mov eax, dword ptr [ebp-14h]
        sub eax, dword ptr [ebp+0Ch]
        js L406F6E
        mov eax, dword ptr [ebp+0Ch]
        sub eax, dword ptr [ebp-0Ch]
        js L406F6E
        mov eax, dword ptr [ebp+0Ch]
        imul dword ptr [ebp-1Ch]
        add eax, dword ptr [ebp-18h]
        add eax, dword ptr [ebp-4]
        mov edi, eax
        mov edx, ecx
        and ecx, 3
        rep movsb
        mov ecx, edx
        shr ecx, 2
        rep movsd
L406F6E:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
BlitRawScanline endp

; IFF chunk identifiers.
g_abIffBmhdChunkId_00406f74 db 'BMHD'
g_abIffCmapChunkId_00406f78 db 'CMAP'
g_abIffBodyChunkId_00406f7c db 'BODY'

; Function start: 0x00406F80
; unsigned char *FindIFFChunkData(const char *chunkId, const unsigned char
;                                 *iffData);
; Finds a chunk in an IFF image.
FindIFFChunkData proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+0Ch]
        add esi, 0Ch
L406F90:
        cmp byte ptr [esi], 0
        jne L406F98
        inc esi
        jmp L406F90
L406F98:
        mov ecx, 2
        mov edi, dword ptr [ebp+8]
        mov eax, esi
        repe cmpsw
        je L406FB9
        mov esi, eax
        add esi, 6
        lodsw
        xchg al, ah
        and eax, 0FFFFh
        add esi, eax
        jmp L406F90
L406FB9:
        add eax, 8
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
FindIFFChunkData endp

; Function start: 0x00406FC2
; unsigned int DecodeIFFImage(RasterClip *clip, const unsigned char *iffData);
; Decodes an IFF ILBM image.
DecodeIFFImage proc
        push ebp
        mov ebp, esp
        add esp, -3Ch
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov edi, dword ptr [ebp+8]
        mov eax, dword ptr [edi+0Ch]
        sub eax, dword ptr [edi+4]
        inc eax
        mov dword ptr [ebp-24h], eax
        mov eax, dword ptr [edi+10h]
        sub eax, dword ptr [edi+8]
        inc eax
        mov dword ptr [ebp-20h], eax
        mov edi, dword ptr [edi]
        mov edi, dword ptr [edi]
        mov dword ptr [ebp-3Ch], edi
        mov dword ptr [ebp-28h], 0
        mov edi, dword ptr [ebp+0Ch]
        mov eax, dword ptr [edi+8]
        xor eax, 4D424C49h
        mov dword ptr [ebp-4], eax
        push dword ptr [ebp+0Ch]
        push offset g_abIffBmhdChunkId_00406f74
        call FindIFFChunkData
        add esp, 8
        mov esi, eax
        lodsw
        xchg al, ah
        and eax, 0FFFFh
        mov dword ptr [ebp-0Ch], eax
        lodsw
        xchg al, ah
        and eax, 0FFFFh
        cmp eax, dword ptr [ebp-20h]
        jl L407031
        mov eax, dword ptr [ebp-20h]
L407031:
        mov dword ptr [ebp-8], eax
        add esi, 5
        lodsb
        cmp al, 1
        je L407180
        mov eax, 0
        lodsb
        mov dword ptr [ebp-2Ch], eax
        add esi, 2
        mov eax, 0
        lodsb
        mov dword ptr [ebp-38h], eax
        mov eax, dword ptr [ebp-0Ch]
        mov ebx, eax
        shr eax, 3
        and ebx, 7
        cmp ebx, 1
        sbb eax, -1
        mov ebx, eax
        and eax, 1
        add ebx, eax
        mov dword ptr [ebp-30h], ebx
        mov eax, dword ptr [ebp-0Ch]
        and eax, 1
        add eax, dword ptr [ebp-0Ch]
        mov dword ptr [ebp-34h], eax
        mov eax, dword ptr [ebp-0Ch]
        cmp eax, dword ptr [ebp-24h]
        jl L407087
        mov eax, dword ptr [ebp-24h]
L407087:
        mov dword ptr [ebp-0Ch], eax
        push dword ptr [ebp+0Ch]
        push offset g_abIffBodyChunkId_00406f7c
        call FindIFFChunkData
        add esp, 8
        mov dword ptr [ebp-10h], eax
L40709D:
        mov esi, dword ptr [ebp-10h]
        cmp dword ptr [ebp-2Ch], 1
        jne L4070F8
        mov edi, offset g_abRasterPaletteUsageScratch_004914f1
        mov edx, dword ptr [ebp-34h]
        add edx, edi
L4070B0:
        cmp edi, edx
        jae L4070EE
        lodsb
        movzx ecx, al
        cmp ecx, 80h
        je L4070B0
        ja L4070D1
        inc ecx
        push ecx
        and ecx, 3
        rep movsb
        pop ecx
        shr ecx, 2
        rep movsd
        jmp L4070B0
L4070D1:
        lodsb
        mov ah, al
        mov ebx, eax
        shl eax, 10h
        mov ax, bx
        neg cl
        inc cl
        push ecx
        and ecx, 3
        rep stosb
        pop ecx
        shr ecx, 2
        rep stosd
        jmp L4070B0
L4070EE:
        mov dword ptr [ebp-10h], esi
        mov esi, offset g_abRasterPaletteUsageScratch_004914f1
        jmp L407100
L4070F8:
        mov eax, esi
        add eax, dword ptr [ebp-34h]
        mov dword ptr [ebp-10h], eax
L407100:
        cmp dword ptr [ebp-4], 0
        jne L407162
        mov edi, offset g_abRasterDecodeBuffer_004907f1
        mov eax, dword ptr [ebp-30h]
        mov dword ptr [ebp-18h], eax
        mov dword ptr [ebp-1Ch], eax
        mov eax, dword ptr [ebp-0Ch]
        mov dword ptr [ebp-14h], eax
L40711A:
        mov edx, 80h
L40711F:
        mov ebx, 0
        mov eax, 100h
L407129:
        movzx ecx, byte ptr [esi+ebx]
        and ecx, edx
        je L407133
        or al, ah
L407133:
        add ebx, dword ptr [ebp-1Ch]
        shl ah, 1
        jne L407129
        stosb
        dec dword ptr [ebp-14h]
        je L40714A
        shr dl, 1
        jne L40711F
        inc esi
        dec dword ptr [ebp-18h]
        jne L40711A
L40714A:
        push dword ptr [ebp-0Ch]
        push offset g_abRasterDecodeBuffer_004907f1
        push dword ptr [ebp-28h]
        push dword ptr [ebp+8]
        call BlitRawScanline
        add esp, 10h
        jmp L407174
L407162:
        push dword ptr [ebp-0Ch]
        push esi
        push dword ptr [ebp-28h]
        push dword ptr [ebp+8]
        call BlitRawScanline
        add esp, 10h
L407174:
        inc dword ptr [ebp-28h]
        dec dword ptr [ebp-8]
        jne L40709D
L407180:
        mov eax, dword ptr [ebp-38h]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DecodeIFFImage endp

; Function start: 0x00407189
; void CopyILBMPalette(const unsigned char *iffData, unsigned char *palette);
; Copies the palette of an ILBM image.
CopyILBMPalette proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        push dword ptr [ebp+8]
        push offset g_abIffCmapChunkId_00406f78
        call FindIFFChunkData
        add esp, 8
        mov esi, eax
        mov edi, dword ptr [ebp+0Ch]
        mov ecx, 300h
L4071AD:
        lodsb
        shr al, 2
        stosb
        loop L4071AD
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CopyILBMPalette endp

; Function start: 0x004071BA
; unsigned int GetILBMImageSize(const unsigned char *iffData);
; Returns the size of an ILBM image.
GetILBMImageSize proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        push dword ptr [ebp+8]
        push offset g_abIffBmhdChunkId_00406f74
        call FindIFFChunkData
        add esp, 8
        mov esi, eax
        lodsw
        xchg al, ah
        shl eax, 10h
        lodsw
        xchg al, ah
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetILBMImageSize endp

; Function start: 0x004071E7
; unsigned int DecodePCXImage(RasterClip *clip, const unsigned char *pcxData);
; Decodes a PCX image.
DecodePCXImage proc
        push ebp
        mov ebp, esp
        add esp, -0Ch
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov dword ptr [ebp-0Ch], 0
        mov edi, dword ptr [ebp+8]
        mov edi, dword ptr [edi]
        mov edi, dword ptr [edi]
        mov esi, dword ptr [ebp+0Ch]
        movzx ebx, word ptr [esi+0Ah]
        sub bx, word ptr [esi+6]
        mov dword ptr [ebp-8], ebx
        mov ebx, 0
        movzx eax, word ptr [esi+42h]
        mov dword ptr [ebp-4], eax
        add esi, 80h
L407222:
        mov edi, offset g_abRasterDecodeBuffer_004907f1
        mov edx, edi
        add edx, dword ptr [ebp-4]
L40722C:
        lodsb
        mov ah, al
        and ah, 0C0h
        xor ah, 0C0h
        jne L407241
        and eax, 3Fh
        mov ecx, eax
        lodsb
        rep stosb
        jmp L407242
L407241:
        stosb
L407242:
        cmp edi, edx
        jl L40722C
        push dword ptr [ebp-4]
        push offset g_abRasterDecodeBuffer_004907f1
        push ebx
        push dword ptr [ebp+8]
        call BlitRawScanline
        add esp, 10h
        inc ebx
        cmp ebx, dword ptr [ebp-8]
        jle L407222
        mov eax, dword ptr [ebp-0Ch]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
DecodePCXImage endp

; Function start: 0x00407269
; void CopyPCXPaletteFromFileTail(const unsigned char *fileData, unsigned int
;                                 fileSize, unsigned char *palette);
; Copies the palette stored at the end of a PCX file.
CopyPCXPaletteFromFileTail proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        add esi, dword ptr [ebp+0Ch]
        sub esi, 300h
        mov edi, dword ptr [ebp+10h]
        mov ecx, 300h
L407287:
        lodsb
        shr al, 2
        stosb
        loop L407287
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CopyPCXPaletteFromFileTail endp

; Function start: 0x00407294
; unsigned int GetPCXImageSize(const unsigned char *header);
; Returns the size of a PCX image.
GetPCXImageSize proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        mov ax, word ptr [esi+8]
        sub ax, word ptr [esi+4]
        inc ax
        shl eax, 10h
        mov ax, word ptr [esi+0Ah]
        sub ax, word ptr [esi+6]
        inc ax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetPCXImageSize endp

; Function start: 0x004072BB
; void ResetGIFLZWDictionary(void);
; Resets the LZW dictionary of the GIF decoder.
ResetGIFLZWDictionary proc
        mov ebx, 0
        mov eax, ecx
        add eax, 2
        mov dword ptr [edi], eax
        mov eax, ecx
        shl eax, 1
        mov dword ptr [edi+4], eax
L4072CE:
        cmp ebx, ecx
        jge L4072ED
        mov byte ptr [ebx+edi+102Eh], bl
        mov byte ptr [ebx+edi+202Eh], bl
        mov word ptr [edi+ebx*2+302Eh], 0FFFFh
        inc ebx
        jmp L4072CE
L4072ED:
        cmp ebx, 1000h
        jge L407302
        mov word ptr [edi+ebx*2+302Eh], 0FFFEh
        inc ebx
        jmp L4072ED
L407302:
        ret
ResetGIFLZWDictionary endp

; Function start: 0x00407303
; unsigned int ReadGIFDataSubBlockByte(void);
; Reads one byte from the GIF data sub-blocks.
ReadGIFDataSubBlockByte proc
        cmp dword ptr [edi+10h], 0
        jne L407312
        lodsb
        and eax, 0FFh
        mov dword ptr [edi+10h], eax
L407312:
        lodsb
        and eax, 0FFh
        dec dword ptr [edi+10h]
        ret
ReadGIFDataSubBlockByte endp

; Function start: 0x0040731C
; unsigned int ReadGIFLZWCode(void);
; Reads one LZW code from a GIF stream.
ReadGIFLZWCode proc
        cmp dword ptr [edi+18h], 0
        jne L407331
        call ReadGIFDataSubBlockByte
        mov dword ptr [edi+14h], eax
        mov dword ptr [edi+18h], 8
L407331:
        mov eax, edx
        cmp dword ptr [edi+18h], eax
        jge L407349
        call ReadGIFDataSubBlockByte
        mov ecx, dword ptr [edi+18h]
        shl eax, cl
        or dword ptr [edi+14h], eax
        add dword ptr [edi+18h], 8
L407349:
        mov ebx, edx
        movzx eax, byte ptr [ebx+g_abGifLZWCodeMasks_00491af1]
        mov ebx, dword ptr [edi+14h]
        and ebx, eax
        push ebx
        sub dword ptr [edi+18h], edx
        mov ecx, edx
        shr dword ptr [edi+14h], cl
        pop eax
        ret
ReadGIFLZWCode endp

; Function start: 0x00407362
; void AppendGIFLZWDictionaryEntry(void);
; Appends one entry to the LZW dictionary.
AppendGIFLZWDictionaryEntry proc
        push ebx
        mov ebx, dword ptr [edi]
        mov word ptr [edi+ebx*2+302Eh], cx
        pop ebx
        push ebx
        mov al, byte ptr [ebx+edi+102Eh]
        mov ebx, dword ptr [edi]
        mov byte ptr [ebx+edi+202Eh], al
        mov ebx, ecx
        mov al, byte ptr [ebx+edi+102Eh]
        mov ebx, dword ptr [edi]
        mov byte ptr [ebx+edi+102Eh], al
        pop ebx
        inc dword ptr [edi]
        mov eax, dword ptr [edi]
        cmp eax, dword ptr [edi+4]
        jne L4073A7
        cmp dword ptr [edi+1Ch], 0Ch
        jge L4073A7
        inc dword ptr [edi+1Ch]
        shl dword ptr [edi+4], 1
L4073A7:
        ret
AppendGIFLZWDictionaryEntry endp

; Function start: 0x004073A8
; void EmitGIFDecodedPixel(void);
; Stores one decoded GIF pixel, following the interlace order.
EmitGIFDecodedPixel proc
        mov ebx, dword ptr [edi+8]
        mov byte ptr [ebx+g_abRasterDecodeBuffer_004907f1], al
        inc dword ptr [edi+8]
        dec dword ptr [edi+20h]
        cmp dword ptr [edi+20h], 0
        jne L407424
        push dword ptr [edi+24h]
        push offset g_abRasterDecodeBuffer_004907f1
        push dword ptr [edi+0Ch]
        push dword ptr [g_pGifRasterTarget_00491b04]
        call BlitRawScanline
        add esp, 10h
        mov dword ptr [edi+8], 0
        mov eax, dword ptr [edi+24h]
        mov dword ptr [edi+20h], eax
        cmp byte ptr [edi+2Ch], 0
        je L407412
        movzx ebx, byte ptr [edi+2Dh]
        movzx eax, byte ptr [ebx+g_abGifInterlaceRowSteps_00491afa]
        add dword ptr [edi+0Ch], eax
        mov eax, dword ptr [edi+0Ch]
        cmp eax, dword ptr [edi+28h]
        jl L407410
        inc byte ptr [edi+2Dh]
        movzx ebx, byte ptr [edi+2Dh]
        movzx eax, byte ptr [ebx+g_abGifInterlaceRowStarts_00491aff]
        mov dword ptr [edi+0Ch], eax
L407410:
        jmp L407424
L407412:
        inc dword ptr [edi+0Ch]
        mov eax, dword ptr [edi+0Ch]
        cmp eax, dword ptr [edi+28h]
        jl L407424
        mov dword ptr [edi+0Ch], 0
L407424:
        ret
EmitGIFDecodedPixel endp

; Function start: 0x00407425
; unsigned int ExpandGIFLZWImage(RasterClip *clip, const unsigned char
;                                *gifData, void *workspace);
; Expands the LZW image data of a GIF.
ExpandGIFLZWImage proc
        push ebp
        mov ebp, esp
        add esp, -18h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov edi, dword ptr [ebp+8]
        mov dword ptr [g_pGifRasterTarget_00491b04], edi
        mov edi, dword ptr [ebp+10h]
        mov eax, 0
        mov ecx, 2Eh
        mov edx, ecx
        and ecx, 3
        rep stosb
        mov ecx, edx
        shr ecx, 2
        rep stosd
        mov edi, dword ptr [ebp+10h]
        mov esi, dword ptr [ebp+0Ch]
        movzx eax, byte ptr [esi+0Bh]
        mov dword ptr [ebp-18h], eax
        mov al, byte ptr [esi+0Ah]
        mov cl, al
        and cl, 7
        inc cl
        mov ebx, 1
        shl ebx, cl
        add esi, 0Dh
        test al, 80h
        je L407480
        imul ebx, ebx, 3
        add esi, ebx
L407480:
        movzx eax, word ptr [esi+5]
        mov dword ptr [edi+24h], eax
        movzx eax, word ptr [esi+7]
        mov dword ptr [edi+28h], eax
        movzx eax, byte ptr [esi+9]
        mov byte ptr [edi+2Ch], al
        and byte ptr [edi+2Ch], 40h
        add esi, 0Ah
        test al, 80h
        je L4074B3
        mov cl, al
        and cl, 7
        inc cl
        mov ebx, 1
        shl ebx, cl
        imul ebx, ebx, 3
        add esi, ebx
L4074B3:
        mov dword ptr [edi+10h], 0
        lodsb
        movzx ecx, al
        mov edx, 8
        push ecx
        push edx
        mov eax, 1
        shl eax, cl
        mov dword ptr [ebp-4], eax
        inc eax
        mov dword ptr [ebp-8], eax
        inc ecx
        mov dword ptr [edi+1Ch], ecx
        mov ecx, dword ptr [ebp-4]
        call ResetGIFLZWDictionary
        mov dword ptr [ebp-10h], 0FFFFh
        mov dword ptr [ebp-14h], 0
        mov byte ptr [edi+2Dh], 0
        mov eax, dword ptr [edi+24h]
        mov dword ptr [edi+20h], eax
        mov dword ptr [edi+8], 0
        mov dword ptr [edi+0Ch], 0
        pop edx
        pop ecx
L407507:
        push ecx
        push edx
        mov edx, dword ptr [edi+1Ch]
        cmp edx, 8
        jg L40751A
        push edx
        call ReadGIFLZWCode
        pop edx
        jmp L407537
L40751A:
        push edx
        mov edx, 8
        call ReadGIFLZWCode
        pop edx
        push eax
        push edx
        sub edx, 8
        call ReadGIFLZWCode
        pop edx
        shl eax, 8
        pop ebx
        or eax, ebx
L407537:
        mov dword ptr [ebp-0Ch], eax
        pop edx
        pop ecx
        cmp eax, dword ptr [ebp-4]
        jne L40755F
        push ecx
        push edx
        mov ecx, dword ptr [ebp-4]
        call ResetGIFLZWDictionary
        pop edx
        pop ecx
        mov eax, ecx
        inc eax
        mov dword ptr [edi+1Ch], eax
        mov dword ptr [ebp-10h], 0FFFFh
        jmp L40762A
L40755F:
        cmp eax, dword ptr [ebp-8]
        jne L40758B
L407564:
        cmp dword ptr [edi+10h], 0
        je L407570
        lodsb
        dec dword ptr [edi+10h]
        jmp L407564
L407570:
        lodsb
        and eax, 0FFh
        mov dword ptr [edi+10h], eax
        cmp dword ptr [edi+10h], 0
        jne L407564
        mov dword ptr [ebp-14h], 0FFFFh
        jmp L40762A
L40758B:
        mov ebx, dword ptr [ebp-0Ch]
        cmp word ptr [edi+ebx*2+302Eh], -2
        je L4075B0
        cmp dword ptr [ebp-10h], 0FFFFh
        je L4075AE
        push ecx
        push edx
        mov ecx, dword ptr [ebp-10h]
        call AppendGIFLZWDictionaryEntry
        pop edx
        pop ecx
L4075AE:
        jmp L4075BF
L4075B0:
        push ecx
        push edx
        mov ebx, dword ptr [ebp-10h]
        mov ecx, dword ptr [ebp-10h]
        call AppendGIFLZWDictionaryEntry
        pop edx
        pop ecx
L4075BF:
        push ecx
        push edx
        mov ebx, dword ptr [ebp-0Ch]
        push esi
        mov ecx, 0
        mov esi, edi
        add esi, 2Eh
L4075CF:
        mov al, byte ptr [ebx+edi+202Eh]
        mov byte ptr [esi], al
        inc esi
        inc ecx
        movzx ebx, word ptr [edi+ebx*2+302Eh]
        cmp ebx, 0FFFFh
        jne L4075CF
        cmp edx, 1
        jne L407610
L4075EF:
        dec esi
        mov al, byte ptr [esi]
        and eax, 1
        push ecx
        call EmitGIFDecodedPixel
        pop ecx
        mov al, byte ptr [esi]
        and eax, 0FFh
        shr eax, 1
        push ecx
        call EmitGIFDecodedPixel
        pop ecx
        loop L4075EF
        jmp L407621
L407610:
        dec esi
        mov al, byte ptr [esi]
        and eax, 0FFh
        push ecx
        call EmitGIFDecodedPixel
        pop ecx
        loop L407610
L407621:
        pop esi
        pop edx
        pop ecx
        mov eax, dword ptr [ebp-0Ch]
        mov dword ptr [ebp-10h], eax
L40762A:
        cmp dword ptr [ebp-14h], 0
        jne L407635
        jmp L407507
L407635:
        mov eax, dword ptr [ebp-18h]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
ExpandGIFLZWImage endp

; Function start: 0x0040763E
; void CopyGIFPalette(const unsigned char *gifData, unsigned char *palette);
; Copies the palette of a GIF image.
CopyGIFPalette proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        mov al, byte ptr [esi+0Ah]
        add esi, 0Dh
        test al, 80h
        je L407672
        mov cl, al
        and cl, 7
        inc cl
        mov ebx, 1
        shl ebx, cl
        imul ebx, ebx, 3
        mov edi, dword ptr [ebp+0Ch]
        mov ecx, ebx
L40766B:
        lodsb
        shr al, 2
        stosb
        loop L40766B
L407672:
        mov al, byte ptr [esi+9]
        test al, 80h
        je L407699
        mov cl, al
        and cl, 7
        inc cl
        mov ebx, 1
        shl ebx, cl
        imul ebx, ebx, 3
        add esi, 0Ah
        mov edi, dword ptr [ebp+0Ch]
        mov ecx, ebx
L407692:
        lodsb
        shr al, 2
        stosb
        loop L407692
L407699:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CopyGIFPalette endp

; Function start: 0x0040769F
; unsigned int GetGIFImageSize(const unsigned char *gifData);
; Returns the size of a GIF image.
GetGIFImageSize proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        mov al, byte ptr [esi+0Ah]
        mov cl, al
        and cl, 7
        inc cl
        mov ebx, 1
        shl ebx, cl
        add esi, 0Dh
        test al, 80h
        je L4076C6
        imul ebx, ebx, 3
        add esi, ebx
L4076C6:
        mov ax, word ptr [esi+5]
        shl eax, 10h
        mov ax, word ptr [esi+7]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetGIFImageSize endp

; Function start: 0x004076D7
; unsigned int GetRLEFrameDimensions(unsigned char *shape, int frame);
; Returns the dimensions of an RLE frame.
GetRLEFrameDimensions proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        add esi, 8
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 3
        add esi, eax
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRLEFrameDimensions endp

; Function start: 0x004076F9
; unsigned int GetRLEFrameExtents(unsigned char *shape, int frame);
; Returns the extents of an RLE frame.
GetRLEFrameExtents proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        add esi, 8
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 3
        add esi, eax
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+4]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRLEFrameExtents endp

; Function start: 0x0040771C
; unsigned int GetRLEImageSize(unsigned char *shape, int frame);
; Returns the size of one RLE image frame.
GetRLEImageSize proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        add esi, 8
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 3
        add esi, eax
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+10h]
        sub eax, dword ptr [esi+8]
        inc eax
        mov ebx, dword ptr [esi+14h]
        sub ebx, dword ptr [esi+0Ch]
        inc ebx
        shl eax, 10h
        mov ax, bx
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRLEImageSize endp

; Function start: 0x00407750
; unsigned int GetRLEImageOrigin(unsigned char *shape, int frame);
; Returns the origin of one RLE image frame.
GetRLEImageOrigin proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        add esi, 8
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 3
        add esi, eax
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+8]
        shl eax, 10h
        mov ax, word ptr [esi+0Ch]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRLEImageOrigin endp

; Function start: 0x0040777A
; void ApplyRLEFramePalette(unsigned char *shape, int frame, unsigned char
;                           *palette);
; Applies the palette stored with an RLE frame.
ApplyRLEFramePalette proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        add esi, 8
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 3
        add esi, eax
        add esi, 4
        mov esi, dword ptr [esi]
        cmp esi, 0
        je L4077BF
        add esi, dword ptr [ebp+8]
        lodsd
        mov ecx, eax
        mov edi, dword ptr [ebp+10h]
L4077A5:
        lodsb
        and eax, 0FFh
        mov ebx, eax
        shl ebx, 1
        add ebx, eax
        lodsb
        mov byte ptr [ebx+edi], al
        inc ebx
        lodsw
        mov word ptr [ebx+edi], ax
        dec ecx
        jne L4077A5
L4077BF:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
ApplyRLEFramePalette endp

; Function start: 0x004077C5
; unsigned int CopyRLEFramePalette(unsigned char *shape, int frame, unsigned
;                                  char *entries);
; Copies the palette stored with an RLE frame.
CopyRLEFramePalette proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        add esi, 8
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 3
        add esi, eax
        add esi, 4
        mov esi, dword ptr [esi]
        cmp esi, 0
        jne L4077EE
        mov eax, 0
        jmp L407807
L4077EE:
        add esi, dword ptr [ebp+8]
        lodsd
        mov ebx, eax
        mov edi, dword ptr [ebp+10h]
        or edi, edi
        je L407807
        mov ecx, ebx
        mov eax, 0
L407802:
        movsd
        loop L407802
        mov eax, ebx
L407807:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CopyRLEFramePalette endp

; Function start: 0x0040780D
; unsigned int SetRLEFramePalette(unsigned char *shape, int frame, const
;                                 unsigned char *entries);
; Stores a palette with an RLE frame.
SetRLEFramePalette proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp+8]
        add esi, 8
        mov eax, dword ptr [ebp+0Ch]
        shl eax, 3
        add esi, eax
        add esi, 4
        mov esi, dword ptr [esi]
        cmp esi, 0
        jne L407836
        mov eax, 0
        jmp L407851
L407836:
        add esi, dword ptr [ebp+8]
        lodsd
        mov ebx, eax
        mov edi, esi
        mov esi, dword ptr [ebp+10h]
        or esi, esi
        je L407851
        mov ecx, ebx
        mov eax, 0
L40784C:
        movsd
        loop L40784C
        mov eax, ebx
L407851:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
SetRLEFramePalette endp

; Function start: 0x00407857
; unsigned int GetRLEFrameCount(const unsigned char *shape);
; Returns the number of frames in an RLE image.
GetRLEFrameCount proc
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+4]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
GetRLEFrameCount endp

; Function start: 0x0040786A
; int CollectUniqueRLEImageFrames(const unsigned char *shape, unsigned int
;                                 *frames);
; Lists the distinct frames of an RLE image.
CollectUniqueRLEImageFrames proc
        push ebp
        mov ebp, esp
        add esp, -4
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        mov ecx, dword ptr [esi+4]
        dec ecx
        add esi, 8
        mov dword ptr [ebp-4], esi
        mov ebx, 1
        mov edx, dword ptr [ebp+0Ch]
        cmp edx, 0
        je L407897
        mov dword ptr [edx], 0
        add edx, 4
L407897:
        cmp ecx, 0
        je L4078C4
L40789C:
        add esi, 8
        mov eax, dword ptr [esi]
        mov edi, dword ptr [ebp-4]
L4078A4:
        cmp eax, dword ptr [edi]
        je L4078C2
        add edi, 8
        cmp edi, esi
        jl L4078A4
        cmp edx, 0
        je L4078C1
        mov eax, esi
        sub eax, dword ptr [ebp-4]
        shr eax, 3
        mov dword ptr [edx], eax
        add edx, 4
L4078C1:
        inc ebx
L4078C2:
        loop L40789C
L4078C4:
        mov eax, ebx
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CollectUniqueRLEImageFrames endp

; Function start: 0x004078CC
; int CollectUniqueRLEPaletteFrames(const unsigned char *shape, unsigned int
;                                   *frames);
; Lists the distinct palettes of an RLE image.
CollectUniqueRLEPaletteFrames proc
        push ebp
        mov ebp, esp
        add esp, -4
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp+8]
        mov ecx, dword ptr [esi+4]
        dec ecx
        add esi, 0Ch
        mov dword ptr [ebp-4], esi
        mov ebx, 1
        mov edx, dword ptr [ebp+0Ch]
        cmp edx, 0
        je L4078F9
        mov dword ptr [edx], 0
        add edx, 4
L4078F9:
        cmp ecx, 0
        je L407926
L4078FE:
        add esi, 8
        mov eax, dword ptr [esi]
        mov edi, dword ptr [ebp-4]
L407906:
        cmp eax, dword ptr [edi]
        je L407924
        add edi, 8
        cmp edi, esi
        jl L407906
        cmp edx, 0
        je L407923
        mov eax, esi
        sub eax, dword ptr [ebp-4]
        shr eax, 3
        mov dword ptr [edx], eax
        add edx, 4
L407923:
        inc ebx
L407924:
        loop L4078FE
L407926:
        mov eax, ebx
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CollectUniqueRLEPaletteFrames endp

; Maximal-length LFSR feedback masks for DissolveRasterClip, indexed by
; register width from 2.
g_adwRasterDissolveTaps_0040792e dd 3, 6, 0Ch, 14h, 30h, 60h, 0B8h, 110h
                                 dd 240h, 500h, 0CA0h, 1B00h, 3500h, 6000h, 0B400h, 12000h
                                 dd 20400h, 72000h, 90000h, 140000h, 300000h, 420000h, 0D80000h, 1200000h
                                 dd 3880000h, 7200000h, 9000000h, 14000000h, 32800000h, 48000000h, 0A3000000h

; Function start: 0x004079AA
; Unreferenced. Copies pixels from one clip to another in the order of a
; maximal-length LFSR, a dissolve that a caller can resume from the
; returned register state.  The tap table before it holds one feedback
; mask per register width.
DissolveRasterClip proc
        push ebp
        mov ebp, esp
        add esp, -70h
        push es
        push ebx
        push esi
        push edi
        mov esi, dword ptr [ebp+8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-44h], eax
        jle L407A26
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L407A26
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-4Ch], eax
        cmp eax, 0
        jg L4079DA
        mov eax, 0
L4079DA:
        mov dword ptr [ebp-34h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-50h], eax
        cmp eax, 0
        jg L4079ED
        mov eax, 0
L4079ED:
        mov dword ptr [ebp-38h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-44h]
        dec edx
        cmp eax, edx
        jl L4079FD
        mov eax, edx
L4079FD:
        mov dword ptr [ebp-3Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L407A0C
        mov eax, edx
L407A0C:
        mov dword ptr [ebp-40h], eax
        mov eax, dword ptr [ebp-3Ch]
        cmp eax, dword ptr [ebp-34h]
        jl L407A31
        mov eax, dword ptr [ebp-40h]
        cmp eax, dword ptr [ebp-38h]
        jl L407A31
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-48h], eax
        jmp L407A3C
L407A26:
        mov eax, 0FFFFFFFFh
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
L407A31:
        mov eax, 0FFFFFFFEh
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
L407A3C:
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+0Ch]
        sub eax, dword ptr [esi+4]
        inc eax
        mov dword ptr [ebp-24h], eax
        mov eax, dword ptr [esi+10h]
        sub eax, dword ptr [esi+8]
        inc eax
        mov dword ptr [ebp-28h], eax
        mov eax, dword ptr [ebp-50h]
        imul dword ptr [ebp-44h]
        add eax, dword ptr [ebp-48h]
        add eax, dword ptr [ebp-4Ch]
        mov edi, eax
        xor ebx, ebx
        mov ecx, dword ptr [ebp-28h]
L407A66:
        mov dword ptr [ebx*4+g_apRasterDissolveSourceRows_004902f1], edi
        inc ebx
        add edi, dword ptr [ebp-44h]
        loop L407A66
        mov esi, dword ptr [ebp+0Ch]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx+4]
        inc eax
        mov dword ptr [ebp-68h], eax
        jle L407AE5
        mov eax, dword ptr [ebx+8]
        inc eax
        mov ecx, eax
        jle L407AE5
        mov eax, dword ptr [esi+4]
        mov dword ptr [ebp-6Ch], eax
        cmp eax, 0
        jg L407A99
        mov eax, 0
L407A99:
        mov dword ptr [ebp-54h], eax
        mov eax, dword ptr [esi+8]
        mov dword ptr [ebp-70h], eax
        cmp eax, 0
        jg L407AAC
        mov eax, 0
L407AAC:
        mov dword ptr [ebp-58h], eax
        mov eax, dword ptr [esi+0Ch]
        mov edx, dword ptr [ebp-68h]
        dec edx
        cmp eax, edx
        jl L407ABC
        mov eax, edx
L407ABC:
        mov dword ptr [ebp-5Ch], eax
        mov eax, dword ptr [esi+10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl L407ACB
        mov eax, edx
L407ACB:
        mov dword ptr [ebp-60h], eax
        mov eax, dword ptr [ebp-5Ch]
        cmp eax, dword ptr [ebp-54h]
        jl L407AF0
        mov eax, dword ptr [ebp-60h]
        cmp eax, dword ptr [ebp-58h]
        jl L407AF0
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp-64h], eax
        jmp L407AFB
L407AE5:
        mov eax, 0FFFFFFFFh
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
L407AF0:
        mov eax, 0FFFFFFFEh
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
L407AFB:
        mov esi, dword ptr [ebp+0Ch]
        mov eax, dword ptr [esi+0Ch]
        sub eax, dword ptr [esi+4]
        inc eax
        mov dword ptr [ebp-2Ch], eax
        mov eax, dword ptr [esi+10h]
        sub eax, dword ptr [esi+8]
        inc eax
        mov dword ptr [ebp-30h], eax
        mov eax, dword ptr [ebp-70h]
        imul dword ptr [ebp-68h]
        add eax, dword ptr [ebp-64h]
        add eax, dword ptr [ebp-6Ch]
        mov edi, eax
        xor ebx, ebx
        mov ecx, dword ptr [ebp-30h]
L407B25:
        mov dword ptr [ebx*4+g_abRasterDecodeBuffer_004907f1+700h], edi
        inc ebx
        add edi, dword ptr [ebp-68h]
        loop L407B25
        mov eax, dword ptr [ebp-28h]
        cmp eax, dword ptr [ebp-30h]
        jl L407B3D
        mov eax, dword ptr [ebp-30h]
L407B3D:
        mov dword ptr [ebp-4], eax
        mov eax, dword ptr [ebp-24h]
        mov ebx, dword ptr [ebp-2Ch]
        cmp ebx, eax
        jg L407B4C
        mov eax, ebx
L407B4C:
        mov dword ptr [ebp-8], eax
        xor ebx, ebx
        mov dword ptr [ebp-20h], ebx
        mov eax, dword ptr [ebp-4]
L407B57:
        inc ebx
        stc
        rcl dword ptr [ebp-20h], 1
        shr eax, 1
        jne L407B57
        mov dword ptr [ebp-1Ch], ebx
        mov eax, dword ptr [ebp-8]
L407B66:
        inc ebx
        shr eax, 1
        jne L407B66
        mov eax, dword ptr [ebx*4+g_adwRasterDissolveTaps_0040792e-8]
        mov dword ptr [ebp-10h], eax
        cmp dword ptr [ebp+14h], 0
        jne L407B8E
        mov dword ptr [ebp+14h], 1
        mov ebx, 0
        mov esi, 0
        jmp L407BBC
L407B8E:
        mov esi, dword ptr [ebp+14h]
        shr esi, 1
        jae L407B98
        xor esi, dword ptr [ebp-10h]
L407B98:
        mov dword ptr [ebp+14h], esi
        cmp esi, 1
        jne L407BA7
        mov dword ptr [ebp+10h], 0
L407BA7:
        mov ecx, dword ptr [ebp-1Ch]
        shr esi, cl
        cmp esi, dword ptr [ebp-8]
        jae L407B8E
        mov ebx, dword ptr [ebp+14h]
        and ebx, dword ptr [ebp-20h]
        cmp ebx, dword ptr [ebp-4]
        jae L407B8E
L407BBC:
        dec dword ptr [ebp+10h]
        mov eax, esi
        add eax, dword ptr [ebp-6Ch]
        cmp eax, dword ptr [ebp-54h]
        jl L407B8E
        cmp eax, dword ptr [ebp-5Ch]
        jg L407B8E
        mov eax, esi
        add eax, dword ptr [ebp-4Ch]
        cmp eax, dword ptr [ebp-34h]
        jl L407B8E
        cmp eax, dword ptr [ebp-3Ch]
        jg L407B8E
        mov eax, ebx
        add eax, dword ptr [ebp-70h]
        cmp eax, dword ptr [ebp-58h]
        jl L407B8E
        cmp eax, dword ptr [ebp-60h]
        jg L407B8E
        mov eax, ebx
        add eax, dword ptr [ebp-50h]
        cmp eax, dword ptr [ebp-38h]
        jl L407B8E
        cmp eax, dword ptr [ebp-40h]
        jg L407B8E
        mov edi, dword ptr [ebx*4+g_abRasterDecodeBuffer_004907f1+700h]
        add edi, esi
        mov eax, dword ptr [ebx*4+g_apRasterDissolveSourceRows_004902f1]
        add esi, eax
        movsb
        cmp dword ptr [ebp+10h], 0
        jge L407B8E
        mov eax, dword ptr [ebp+14h]
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
DissolveRasterClip endp

; Function start: 0x00407C21
; void FadeRasterPaletteToPalette(RasterSurface *surface, const unsigned char
;                                 *targetPalette, unsigned int duration);
; Fades a surface palette towards another in fixed point, through the
; driver callbacks.
FadeRasterPaletteToPalette proc
        push ebp
        mov ebp, esp
        add esp, -10h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov edi, offset g_abRasterPaletteUsageScratch_004914f1
        mov eax, 0
        mov ecx, 40h
        rep stosd
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+4]
        inc eax
        mov ecx, dword ptr [esi+8]
        inc ecx
        imul ecx, eax
        mov esi, dword ptr [esi]
        mov edi, offset g_abRasterDecodeBuffer_004907f1+300h
        mov dword ptr [ebp-4], 0FFFFFFFFh
L407C5B:
        mov eax, 0
        lodsb
        cmp byte ptr [eax+g_abRasterPaletteUsageScratch_004914f1], 0
        jne L407C8E
        or byte ptr [eax+g_abRasterPaletteUsageScratch_004914f1], 1
        stosb
        inc dword ptr [ebp-4]
        mov ebx, eax
        shl ebx, 1
        add ebx, eax
        add ebx, offset g_abRasterDecodeBuffer_004907f1
        push ecx
        push ebx
        push eax
        call dword ptr [g_apRasterDriverCallbacks_00490278+20h]
        add esp, 8
        pop ecx
L407C8E:
        dec ecx
        jne L407C5B
        mov dword ptr [ebp-8], 0
        mov ecx, dword ptr [ebp-4]
        mov ebx, 0
        mov esi, dword ptr [ebp+0Ch]
L407CA3:
        movzx eax, byte ptr [ecx+g_abRasterDecodeBuffer_004907f1+300h]
        mov ebx, eax
        shl ebx, 1
        add ebx, eax
        mov edx, 2
L407CB5:
        mov ah, 0FFh
        mov al, byte ptr [ebx+g_abRasterDecodeBuffer_004907f1]
        sub al, byte ptr [ebx+esi]
        jge L407CC6
        mov ah, 1
        neg al
L407CC6:
        mov byte ptr [ebx+g_abRasterPaletteUsageScratch_004914f1], ah
        mov byte ptr [ebx+g_abRasterDecodeBuffer_004907f1+400h], al
        cmp al, byte ptr [ebp-8]
        jle L407CDA
        mov byte ptr [ebp-8], al
L407CDA:
        inc ebx
        dec edx
        jns L407CB5
        dec ecx
        jns L407CA3
        mov ecx, dword ptr [ebp-4]
        mov edi, offset g_abRasterFadeAccumulator_004917f1
        mov al, byte ptr [ebp-8]
        shr al, 1
        mov ah, al
        shr ecx, 1
        rep stosw
        adc ecx, 0
        rep stosb
        movzx esi, byte ptr [ebp-8]
        cmp esi, 0
        je L407D9F
        mov eax, dword ptr [ebp+10h]
        mov edx, 0
        shld edx, eax, 10h
        shl eax, 10h
        div esi
        mov dword ptr [ebp-0Ch], eax
        mov dword ptr [ebp-10h], 8000h
L407D22:
        mov ecx, dword ptr [ebp-4]
L407D25:
        movzx eax, byte ptr [ecx+g_abRasterDecodeBuffer_004907f1+300h]
        mov ebx, eax
        shl ebx, 1
        add ebx, eax
        mov edx, 2
L407D37:
        mov al, byte ptr [edx+ebx+g_abRasterFadeAccumulator_004917f1]
        add al, byte ptr [edx+ebx+g_abRasterDecodeBuffer_004907f1+400h]
        cmp al, byte ptr [ebp-8]
        jl L407D5B
        sub al, byte ptr [ebp-8]
        mov ah, byte ptr [edx+ebx+g_abRasterPaletteUsageScratch_004914f1]
        add byte ptr [edx+ebx+g_abRasterDecodeBuffer_004907f1], ah
L407D5B:
        mov byte ptr [edx+ebx+g_abRasterFadeAccumulator_004917f1], al
        dec edx
        jns L407D37
        push ecx
        mov eax, ebx
        add eax, offset g_abRasterDecodeBuffer_004907f1
        push eax
        movzx eax, byte ptr [ecx+g_abRasterDecodeBuffer_004907f1+300h]
        push eax
        call dword ptr [g_apRasterDriverCallbacks_00490278+24h]
        add esp, 8
        pop ecx
        dec ecx
        jns L407D25
        mov eax, dword ptr [ebp-0Ch]
        add dword ptr [ebp-10h], eax
        cmp word ptr [ebp-0Eh], 1
        jl L407D9C
L407D90:
        call dword ptr [g_apRasterDriverCallbacks_00490278+14h]
        dec word ptr [ebp-0Eh]
        jne L407D90
L407D9C:
        dec esi
        jne L407D22
L407D9F:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
FadeRasterPaletteToPalette endp

; Function start: 0x00407DA5
; int CollectRasterClipColours(RasterClip *clip, unsigned int *colours);
; Unreferenced. Collects the set of colours used in a clip.
CollectRasterClipColours proc
        push ebp
        mov ebp, esp
        add esp, -0Ch
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov edi, offset g_abRasterPaletteUsageScratch_004914f1
        mov eax, 0
        mov ecx, 40h
        rep stosd
        mov esi, dword ptr [ebp+8]
        mov ecx, dword ptr [esi+0Ch]
        sub ecx, dword ptr [esi+4]
        mov dword ptr [ebp-8], ecx
        mov ebx, dword ptr [esi+10h]
        sub ebx, dword ptr [esi+8]
        mov esi, dword ptr [esi]
        mov eax, dword ptr [esi+4]
        inc eax
        mov dword ptr [ebp-0Ch], eax
        mov esi, dword ptr [ebp+8]
        mov eax, dword ptr [esi+8]
        imul dword ptr [ebp-0Ch]
        add eax, dword ptr [esi+4]
        mov esi, dword ptr [esi]
        mov esi, dword ptr [esi]
        add esi, eax
        mov edi, dword ptr [ebp+0Ch]
        mov dword ptr [ebp-4], 0FFFFFFFFh
L407DFA:
        mov eax, 0
        movzx eax, byte ptr [ecx+esi]
        cmp byte ptr [eax+g_abRasterPaletteUsageScratch_004914f1], 0
        jne L407E1C
        or byte ptr [eax+g_abRasterPaletteUsageScratch_004914f1], 1
        inc dword ptr [ebp-4]
        cmp edi, 0
        je L407E1C
        stosd
L407E1C:
        dec ecx
        jns L407DFA
        mov ecx, dword ptr [ebp-8]
        add esi, dword ptr [ebp-0Ch]
        dec ebx
        jns L407DFA
        mov eax, dword ptr [ebp-4]
        inc eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
CollectRasterClipColours endp

        .data

g_apRasterDriverCallbacks_00490278 dd 0Dh dup (0)
g_szRasterDriverName_004902ac db 'MCGA.DLL', 5 dup (0)
g_pRLEEncodeBuffer_004902b9 dd 0
g_nRLEPendingSkip_004902bd dd 0
g_pRLEScanlineStart_004902c1 dd 0
g_pRLEScanCursor_004902c5 dd 0
g_pRLEOutputCursor_004902c9 dd 0
g_pRLELiteralStart_004902cd dd 0
; Unreferenced.
        db 10h dup (0)
g_nRLEEncodedMinimumX_004902e1 dd 0
g_nRLEEncodedMinimumY_004902e5 dd 0
g_nRLEEncodedMaximumX_004902e9 dd 0
g_nRLEEncodedMaximumY_004902ed dd 0
; Source row addresses for DissolveRasterClip.
g_apRasterDissolveSourceRows_004902f1 dd 140h dup (0)
; Image decode buffer; DissolveRasterClip keeps its target row
; addresses at +700h.
g_abRasterDecodeBuffer_004907f1 db 0D00h dup (0)
g_abRasterPaletteUsageScratch_004914f1 db 300h dup (0)
g_abRasterFadeAccumulator_004917f1 db 300h dup (0)
g_abGifLZWCodeMasks_00491af1 db 0, 1, 3, 7, 0Fh, 1Fh, 3Fh, 7Fh, 0FFh
g_abGifInterlaceRowSteps_00491afa db 8, 8, 4, 2, 0
g_abGifInterlaceRowStarts_00491aff db 0, 4, 2, 1, 0
g_pGifRasterTarget_00491b04 dd 0
g_abRasterPaletteTranslation_00491b08 db 100h dup (0)
g_aRLETransformVertices_00491c08 dd 14h dup (0)
g_anRLESourceSteps_00491c58 dd 4 dup (0)

        end
