/*
 * Copyright (C) 2012 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef LINKER_PHDR_H
#define LINKER_PHDR_H

/* Declarations related to the ELF program header table and segments.
 *
 * The design goal is to provide an API that is as close as possible
 * to the ELF spec, and does not depend on linker-specific data
 * structures (e.g. the exact layout of struct soinfo).
 */

#include <stddef.h>
#include "sys/exec_elf.h"
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
// Returns the address of the page containing address 'x'.
#define PAGE_START(x)  ((x) & PAGE_MASK)

// Returns the offset of address 'x' in its page.
#define PAGE_OFFSET(x) ((x) & ~PAGE_MASK)

// Returns the address of the next page after address 'x', unless 'x' is
// itself at the start of a page.
#define PAGE_END(x)    PAGE_START((x) + (PAGE_SIZE-1))

#ifdef __cplusplus
extern "C" {
#endif

extern void setTextView(unsigned char *a);

#ifdef __cplusplus
}
#endif

class ElfReader {
public:
    ElfReader(const char *name, int fd);

    ~ElfReader();

    bool Load();

    size_t phdr_count() { return phdr_num_; }

    Elf32_Addr load_start() { return reinterpret_cast<Elf32_Addr>(load_start_); }

    Elf32_Addr load_size() { return load_size_; }

    Elf32_Addr load_bias() { return load_bias_; }

    const Elf32_Phdr *loaded_phdr() { return loaded_phdr_; }

private:
    bool ReadElfHeader();

    bool VerifyElfHeader();

    bool ReadProgramHeader();

    bool ReserveAddressSpace();

    bool LoadSegments();

    bool FindPhdr();

    bool CheckPhdr(Elf32_Addr);

    void DisplayMmap(const char *name_, unsigned int f_start, unsigned int f_end,
                     unsigned int m_start, unsigned int m_end);

    const char *name_;
    int fd_;

    Elf32_Ehdr header_;
    size_t phdr_num_;

    void *phdr_mmap_;
    Elf32_Phdr *phdr_table_;
    Elf32_Addr phdr_size_;

    // First page of reserved address space.
    void *load_start_;
    // Size in bytes of reserved address space.
    Elf32_Addr load_size_;
    // Load bias.
    Elf32_Addr load_bias_;

    // Loaded phdr.
    const Elf32_Phdr *loaded_phdr_;
};

size_t phdr_table_get_load_size(const Elf32_Phdr *phdr_table,
                                size_t phdr_count,
                                Elf32_Addr *min_vaddr = NULL,
                                Elf32_Addr *max_vaddr = NULL);

int phdr_table_protect_segments(const Elf32_Phdr *phdr_table,
                                int phdr_count,
                                Elf32_Addr load_bias);

int phdr_table_unprotect_segments(const Elf32_Phdr *phdr_table,
                                  int phdr_count,
                                  Elf32_Addr load_bias);

int phdr_table_protect_gnu_relro(const Elf32_Phdr *phdr_table,
                                 int phdr_count,
                                 Elf32_Addr load_bias);

#if 0
#define LOG_BUF_SIZE 160

inline void DL_DBG(const char *fmt, ...) {
    va_list ap;
    char buf[LOG_BUF_SIZE];
    memset(buf, 0, LOG_BUF_SIZE);
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);
    setTextView((unsigned char *) buf);
}

inline void DL_ERR(const char *fmt, ...) {
    va_list ap;
    char buf[LOG_BUF_SIZE];
    memset(buf, 0, LOG_BUF_SIZE);
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);
    setTextView((unsigned char *) buf);
}

#else
#define DL_ERR(...) \
    __android_log_print(ANDROID_LOG_ERROR, "keymatch", __VA_ARGS__)
#define DL_DBG(...) \
    __android_log_print(ANDROID_LOG_DEBUG, "keymatch", __VA_ARGS__)

#define LOG_BUF_SIZE 160

inline void TV_DBG(const char *fmt, ...) {
    va_list ap;
    char buf[LOG_BUF_SIZE];
    memset(buf, 0, LOG_BUF_SIZE);
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);
    setTextView((unsigned char *) buf);
    DL_DBG("%s", buf);
}

inline void TV_ERR(const char *fmt, ...) {
    va_list ap;
    char buf[LOG_BUF_SIZE];
    memset(buf, 0, LOG_BUF_SIZE);
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);
    setTextView((unsigned char *) buf);
    DL_ERR("%s", buf);
}

#endif

#define GETOFFSET(memaddr) ((int) memaddr) - ((int) (si->base))

int phdr_table_get_arm_exidx(const Elf32_Phdr *phdr_table,
                             int phdr_count,
                             Elf32_Addr load_bias,
                             Elf32_Addr **arm_exidx,
                             unsigned *arm_exidix_count);

void phdr_table_get_dynamic_section(const Elf32_Phdr *phdr_table,
                                    int phdr_count,
                                    Elf32_Addr load_bias,
                                    Elf32_Dyn **dynamic,
                                    size_t *dynamic_count,
                                    Elf32_Word *dynamic_flags);

#endif /* LINKER_PHDR_H */
