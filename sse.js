module.exports = function (req, res, next) {
  res.sseSetup = function() {
    res.writeHead(200, {
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      'Connection': 'keep-alive',
      'access-control-allow-origin': '*'
    });
  };

  res.sseSend = function(event, data) {
    res.write(`event:${event}\ndata: ${JSON.stringify(data)}\n\n`);
  };

  next();
};