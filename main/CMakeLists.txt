set(COMPONENT_SRCS "main.c" "ui.c" "wifi.c" "set.c" "bl.c")
set(COMPONENT_ADD_INCLUDEDIRS "./includes")
set(COMPONENT_REQUIRES "bt" "nvs_flash" "esp-aws-iot" "esp-cryptoauthlib" "core2forAWS")
register_component()

target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
