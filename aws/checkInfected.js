const AWS = require("aws-sdk");

const dynamo = new AWS.DynamoDB.DocumentClient();
const iotdata = new AWS.IotData({ endpoint: 'a29ug6stqo7e9n-ats.iot.us-west-2.amazonaws.com' });

exports.handler = async (event) => {
    console.info("EVENT\n" + JSON.stringify(event, null, 2));

    const regex = /([\w]+)\/check/g;
    var client_id = regex.exec(event.topic)[1];

    var item = await dynamo
        .get({
            TableName: "health_alarm_infected",
            Key: {
                own: event.own
            }
        })
        .promise();

    if (item.Item) {
        var params = {
            topic: String(event.topic) + "_response",
            payload: JSON.stringify({
                client_id: client_id,
                timestamp: item.Item.timestamp,
                own: item.Item.own,
                infected: true
            }, null, 2),
            qos: 0
        };
        console.info(params);
        await iotdata.publish(params).promise();
        return {
            client_id: client_id,
            infected: true
        };
    } else {
        return {
            client_id: client_id,
            infected: false
        };
    }
};