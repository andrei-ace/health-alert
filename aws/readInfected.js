const AWS = require("aws-sdk");

const dynamo = new AWS.DynamoDB.DocumentClient();

exports.handler = async (event) => {
    console.info("EVENT\n" + JSON.stringify(event, null, 2));
    const regex = /([\w]+)\/infected/g;
    var client_id = regex.exec(event.topic)[1];

    await dynamo
        .put({
            TableName: "health_alarm_infected",
            Item: {
                own: event.own,
                timestamp: event.timestamp
            }
        })
        .promise();

    var owner = await dynamo.query({
        TableName: "health_alarm_seen",
        KeyConditionExpression: "own = :own",
        ExpressionAttributeValues: {
            ":own": String(event.own)
        }
    }).promise();

    for (var item in owner.Items) {
        await dynamo
            .put({
                TableName: "health_alarm_infected",
                Item: {
                    own: owner.Items[item].seen,
                    timestamp: event.timestamp
                }
            })
            .promise();
    }

    var seen = await dynamo.query({
        TableName: "health_alarm_seen",
        IndexName: "seen-own-index",
        KeyConditionExpression: "seen = :own",
        ExpressionAttributeValues: {
            ":own": String(event.own)
        }
    }).promise();

    for (var item in seen.Items) {
        await dynamo
            .put({
                TableName: "health_alarm_infected",
                Item: {
                    own: seen.Items[item].own,
                    timestamp: event.timestamp
                }
            })
            .promise();
    }
    return {
    };
};