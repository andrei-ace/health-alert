const AWS = require("aws-sdk");

const dynamo = new AWS.DynamoDB.DocumentClient();

exports.handler = async (event) => {
    console.info("EVENT\n" + JSON.stringify(event, null, 2));
    const regex = /([\w]+)\/seen/g;
    var client_id = regex.exec(event.topic)[1];
    for (const addr_seen in event.seen) {
        await dynamo
            .put({
                TableName: "health_alarm_seen",
                Item: {
                    own: event.own,
                    seen: event.seen[addr_seen],
                    timestamp: event.timestamp,
                    client_id: client_id
                }
            })
            .promise();
    }

    return {

    };
};