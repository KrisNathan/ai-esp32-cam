FROM python:3.12-bookworm

RUN apt-get update -y && apt-get install -y \
  ffmpeg \
  libsm6 \
  libxext6 
RUN pip install poetry

WORKDIR /opt/app
COPY . .
RUN poetry install

CMD ["poetry", "run", "uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8080"]