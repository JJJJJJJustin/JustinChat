const path = require('path');
const grpc = require('@grpc/grpc-js');
const protoLoader = require('@grpc/proto-loader');

const PROTO_PATH = path.join(__dirname, 'message.proto');
const packageDifinition = protoLoader.loadSync(PROTO_PATH, {keepCase:true, longs:String, enums:String, defaults:true, oneofs:true});
const protoDescripter = grpc.loadPackageDefinition(packageDifinition);

const messageProto = protoDescripter.message;

module.exports = messageProto;