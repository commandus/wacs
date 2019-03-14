#!/bin/sh
ORG="/C=RU/ST=Sakha/O=commandus.com/L=Internet/OU=IT/CN=andrei"
OUT=..
NAME=wacs-http
PWD=1234567890
# LOG=gen-cert.log
LOG=/dev/null
openssl genrsa -out $OUT/$NAME.key -passout pass:$PWD 1024 2>>$LOG 
openssl req -days 365 -out $OUT/$NAME.pem -new -x509 -key $OUT/$NAME.key -passin pass:$PWD -subj $ORG 
