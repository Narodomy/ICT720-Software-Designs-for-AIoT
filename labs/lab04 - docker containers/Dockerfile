FROM python:3.9

WORKDIR /python-docker
COPY requirement.txt ./
RUN pip install -r requirement.txt
COPY . .
EXPOSE 8080
CMD ["python", "-u", "app.py"]
