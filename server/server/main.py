from fastapi import FastAPI
from pydantic import BaseModel
from ultralytics import YOLO
from PIL import Image
from io import BytesIO
import base64
import fs

model = YOLO('yolov8n.pt')
mem_fs = fs.open_fs('mem://')

app = FastAPI()

class Base64Image(BaseModel):
    img: str

@app.post("/image_upload")
async def create_image_upload(b64img: Base64Image):
    im = Image.open(BytesIO(base64.b64decode(b64img.img)))
    results = model.predict(source=im)  # save plotted images
    print(results)
    for r in results:
        im_array = r.plot()  # plot a BGR numpy array of predictions
        im = Image.fromarray(im_array[..., ::-1])  # RGB PIL image

        buffered = BytesIO()
        im.save(buffered, format="JPEG")
        img_str = base64.b64encode(buffered.getvalue()).decode('utf-8')

        with mem_fs.open('product.txt', 'w+') as mfs:
            mfs.write(img_str)

@app.get("/img")
async def create_image_get():
    txt = ""
    with mem_fs.open('product.txt', 'r') as mfs:
        txt = mfs.read()
    return f"data:image/jpeg;base64,{txt}"

from fastapi.staticfiles import StaticFiles
app.mount("/", StaticFiles(directory="static", html=True), name="static")
