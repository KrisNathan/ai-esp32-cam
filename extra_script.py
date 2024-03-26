Import("env")
import os

print("hello")
print(os)

def before_build_spiffs(source, target, env):
    print("Building Web App...")
    
    # print("Removing old SPIFFS image...")
    # env.Execute("rm -rf data")

    env.Execute("cd web && npm run build")
    print("Web App built!")

    # print("Copying Web App to SPIFFS...")
    # env.Execute("cp -r ./web/dist data")  #cp -r web/dist data  

env.AddPreAction("$BUILD_DIR/spiffs.bin", before_build_spiffs)