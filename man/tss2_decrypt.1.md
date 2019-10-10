% tss2_decrypt(1) tpm2-tools | General Commands Manual
%
% APRIL 2019

# NAME

**tss2_decrypt**(1) - decrypts data

# SYNOPSIS

**tss2_decrypt** [*OPTIONS*]

# DESCRIPTION

**tss2_decrypt**(1) - Input pointer to data to be decrypted, and a place to put the decrypted data.

# OPTIONS

These are the available options:

  * **-k**, **\--keyPath**=:

    Identifies the decryption key. MUST NOT be NULL.

  * **-c**, **\--cipherText**: \<filename\>

    The JSON-encoded cipherText. MUST NOT be NULL.

  * **-f**, **\--force**:

    Force Overwriting the output file.

  * **-p**, **\--plainText**: \<filename\>

    Returns the decrypted data. MAY be NULL.

[common tss2 options](common/tss2-options.md)

# EXAMPLE

    tss2_decrypt --keyPath HS/SRK/myRSACrypt --cipherText=encrypted.file --plainText=-

# RETURNS

0 on success or 1 on failure.

[footer](common/footer.md)
