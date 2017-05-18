#!/bin/zsh
for code in {000..255}; do
    print -P -- "$code: %K{$code}TestTestTestTestTestTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTestttttttttttttttttttttttttTest%f%K"
done

print -P -- "TestTestTestTestTestTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTesTest0123456789012345678901234567890"
