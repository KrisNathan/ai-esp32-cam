FROM python:3.12-bookworm

RUN pip install poetry

WORKDIR /opt/app
COPY . .
RUN poetry install

CMD ["poetry", "run", "uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8080"]