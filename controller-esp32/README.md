# controller-esp32


**Uplaod Note**: in order for this project to work, you'll need to get the webui files uploaded to the esp32.

1. Go over to the controller-webui folder, run `npm install` and `npx ng build`. That will seed the `~/data/www` in this project with the appropriate files. 
2. In the platform.io plugin, under `Platform` select `Upload Filesystem Image`. Note that this process will fail when you have a serial connection open to the device.
3. Upload the code from this folder normally.
