FROM node:18-alpine

WORKDIR /app

COPY package*.json ./
COPY client/package*.json ./client/
RUN npm install && cd client && npm install

COPY . .
RUN cd client && npm run build

EXPOSE 5000
CMD ["node", "server.js"]
