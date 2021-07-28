function DetailedCell(container, scale = 1.0) {
  this.container = container;
  // set the dimensions and margins of the graph
  this.margin = {top: 10, right: 30, bottom: 30, left: 60},
  this.width = container.width() - this.margin.left - this.margin.right,
  this.height = container.height() - this.margin.top - this.margin.bottom;


  // append the svg object to the body of the page
  this.svg = d3.select(container[0])
    .append("svg")
      .attr("width", this.width + this.margin.left + this.margin.right)
      .attr("height", this.height + this.margin.top + this.margin.bottom)
      //.style("background-color", "#D8D8D8")
    .append("g")
      .attr("transform",
            "translate(" + this.margin.left + "," + this.margin.top + ")");

  this.armgroup = this.svg.append("g")
    .attr("opacity", 0.70);

    this.x = d3.scaleLinear()
            .domain([0,1])
            .range([ 0, this.width ]);
    
    this.y = d3.scaleLinear()
            .domain([0,1])
            .range([ this.height, 0]);
            
    this.scale = scale;
    this.thickness = 80*scale;
    this.time = 0;
    
    /*this.clip = this.svg.append("defs").append("clipPath")
        .attr("id", "round-corner")
        .append("rect")
          .attr("width", 10 )
          .attr("height", 10 )
          .attr("x", 0)
          .attr("y", 0);*/

  // Create the svg:defs element and the main gradient definition.
  var svgDefs = this.svg.append('defs');

  var mainGradient = svgDefs.append('linearGradient')
      .attr('id', 'mainGradient');

  // Create the stops of the main gradient. Each stop will be assigned
  // a class to style the stop using CSS.
  mainGradient.append('stop')
      .attr('class', 'stop-left')
      .attr('offset', '0');

  mainGradient.append('stop')
      .attr('class', 'stop-right')
      .attr('offset', '1');

  this.substrate_force_color = d3.scaleLinear().domain([0,1])
      .range(["#4F67B4", "red"]);
}

DetailedCell.prototype.calcArmLength = function(x1, y1, x2, y2) {
  return Math.sqrt(Math.pow(this.x(x1)-this.x(x2),2) + Math.pow(this.y(y1)-this.y(y2),2));
}

DetailedCell.prototype.calcAflowAnimation = function(d, offsetSpeed = 1.0, offset = 0.0, speed = 1.0) {
  var self = this;
  var armLength = self.calcArmLength(d.x, d.y, self.data.data.x, self.data.data.y);
  var dx = speed*offsetSpeed*10*self.time*self.data.data.aflow/100.0;
  dx = dx - armLength*(Math.floor(dx/armLength));
  var animationPos = armLength*d.pos - dx;
  if (animationPos < 0) {
    animationPos = animationPos + armLength;
  }
  var xVal = self.x(self.data.data.x) + (offset*armLength)+animationPos/offsetSpeed;
  return Number.isNaN(xVal) ? self.x(self.data.data.x): xVal;
}

DetailedCell.prototype.animate = function(dt) {
    var self = this;

    this.time += dt;
    
    if (self.data) {
      this.svg.selectAll(".actin")
        .attr('cx', function(d) {
          return self.calcAflowAnimation(d);
        });

      this.svg.selectAll(".clutch")
        .attr('x1', function(d) {
          return self.calcAflowAnimation(d, 2, 0.5);
        })
        .attr('x2', function(d) {
          return self.calcAflowAnimation(d, 2, 0.5);
        })
        .attr('y2', function(d) {return self.y(self.data.data.y)+ (d.on ? self.thickness/4.0 : self.thickness/8.0); })
        //.attr('y2', function(d) {return self.y(self.data.data.y)+ ((Math.random()*d.c_on_rate) > 0.15 ? self.thickness/4.0 : self.thickness/8.0); })

      this.svg.selectAll(".substrate")
      //.attr('x1', function(d) {return self.x(data.data.x) + (0.5)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
      //.attr('x2', function(d) {return self.x(data.data.x) + (0.5 + 0.3)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
          .attr('x1', function(d) {return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5, d.c_on_rate)-25*self.scale;})
          .attr('x2', function(d) {return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5, d.c_on_rate)+25*self.scale;});

      this.svg.selectAll(".substrate-spring")
          /*.attr("stroke-dasharray",function(d) {
            var armLength = self.calcArmLength(d.x, d.y, self.data.data.x, self.data.data.y);
            var pos = self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5, d.c_on_rate);
            var percent = (armLength-(pos-self.x(self.data.data.x)))/armLength;
            return '' + (25*percent) + ' ' + (15*percent);
          })*/
          .attr("stroke", function(d) {
            var armLength = self.calcArmLength(d.x, d.y, self.data.data.x, self.data.data.y);
            var pos = self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5, d.c_on_rate);
            var percent = (armLength-(pos-self.x(self.data.data.x)))/armLength;
            return self.substrate_force_color(percent*d.percentLength*self.data.data.m.length);
            
          })
          /*.attr("opacity",function(d) {
            var armLength = self.calcArmLength(d.x, d.y, self.data.data.x, self.data.data.y);
            var pos = self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5, d.c_on_rate);
            var percent = (armLength-(pos-self.x(self.data.data.x)))/armLength;
            return percent*0.75+0.25;
          })*/
          /*.attr("stroke-width",function(d) {
            var armLength = self.calcArmLength(d.x, d.y, self.data.data.x, self.data.data.y);
            var pos = self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5, d.c_on_rate);
            var percent = (armLength-(pos-self.x(self.data.data.x)))/armLength;
            return 8*self.scale*percent;
          })*/
          .attr('x1', function(d) {return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5, d.c_on_rate);});

      this.svg.selectAll(".substrate-spring-connect")
          .attr('x1', function(d) {
            return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5}, 2, 0.5, d.c_on_rate);
          })
          .attr('x2', function(d) {
            return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5}, 2, 0.5, d.c_on_rate);
          })
    }

}

DetailedCell.prototype.updateData = function(data, bounds, config = 1, reset = true) {
  var self = this;

  var showClutches = config != 0;
  var showActin = config != 0;
  var showSubstrate = config != 0;

  if (this.data && !reset) {
    data = this.data;
  }
  this.data = data;

  var aspectRatio = 1.0*this.width/this.height;
  
  this.x.domain([bounds[0]*aspectRatio, bounds[1]*aspectRatio]);
  this.y.domain([bounds[2], bounds[3]]);

  var color = d3.scaleLinear().domain([0,100])
          .range(["#4F67B4", "#0D025D"]);


    /*//var arms = data.m;
    this.svg.selectAll("line")
      .data(data.data.m)
      .exit()
      .remove()
      this.svg.selectAll("line")
      .data(data.data.m)
      .enter()
      .append("line")
        .attr("fill", "black")
        .attr("stroke", "black")
        //.attr("stroke-width", "100")
        .attr("rx", "15")
      .merge(this.svg.selectAll("line"))
        .attr('x1', function(d) {return self.x(data.data.x); })
        .attr('y1', function(d) {return self.y(data.data.y); })
        //.attr('x2', function(d) {return self.x(data.data.x + d.x);})
        //.attr('y2', function(d) {return self.y(data.data.y + d.y);})
        .attr('x2', function(d) {return self.x(d.x);})
        .attr('y2', function(d) {return self.y(d.y);})*/

    

    this.armgroup.selectAll(".rect1")
      .data(data.data.m)
      .exit()
      .remove()
    this.armgroup.selectAll(".rect1")
      .data(data.data.m)
      .enter()
      .append("rect")
        .attr("class", "rect1")
        //.attr("class", "filled")
        .attr("fill", "lightgrey")
        .attr("stroke", "lightgrey")
        //.attr("stroke-width", "100")
        .attr("rx", "25")
        .attr('height', self.thickness)
        //.style('opacity',0.5)
        //.attr("clip-path", "url(#round-corner)")
      .merge(this.svg.selectAll(".rect1"))
        .attr('x', function(d) {return self.x(data.data.x); })
        .attr('y', function(d) {return self.y(data.data.y); })
        //.attr('fill', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
        //.attr('stroke', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
        .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
        .attr('transform', function(d) { 
          var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
          var opposite = d.y-data.data.y;//-1;
          var adjacent = d.x-data.data.x;//1;
          var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
          if (adjacent < 0) {
            angle += 180;
          }
          return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-self.thickness/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
        })

      this.svg.selectAll(".rect2")
        .data(data.data.m)
        .exit()
        .remove()
      this.svg.selectAll(".rect2")
        .data(data.data.m)
        .enter()
        .append("rect")
          .attr("class", "rect2")
          .attr('height', 5)
        .merge(this.svg.selectAll(".rect2"))
          .attr('x', function(d) {return self.x(data.data.x); })
          .attr('y', function(d) {return self.y(data.data.y); })
          .attr('fill', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
          .attr('stroke', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
          .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
          .attr('transform', function(d) { 
            var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
            var opposite = d.y-data.data.y;//-1;
            var adjacent = d.x-data.data.x;//1;
            var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
            if (adjacent < 0) {
              angle += 180;
            }
            return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-5/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
          })

        var totalModuleLength = 0.0;
        for (var i = 0; i < data.data.m.length; i++) {
          data.data.m[i].length = Math.sqrt(Math.pow(data.data.m[i].x-data.data.x,2) + Math.pow(data.data.m[i].y-data.data.y,2));
          totalModuleLength += data.data.m[i].length;
        }

        var actin = [];
        if (showActin) {
          for (var i = 0; i < data.data.m.length; i++) {
            if (data.data.m[i].length > 0.001) {
              var percent = data.data.m[i].length / totalModuleLength;
              data.data.m[i].percentLength = percent;
              //data.data.m[i].length = Math.sqrt(Math.pow(data.data.m[i].x-data.data.x,2) + Math.pow(data.data.m[i].y-data.data.y,2));
              var numActinVis = Math.floor(percent*data.data.actin/1000/2);
              for (var j = 0; j < numActinVis; j++) {
                var act = {};
                act = Object.assign(act, data.data.m[i]);
                act.pos = 1.0*(j)/(numActinVis);
                actin.push(act);
              }
            }
  
            //          actin.push({})
          }
        }


        this.svg.selectAll(".actin")
          .data(actin)
          .exit()
          .remove()
        this.svg.selectAll(".actin")
          .data(actin)
          .enter()
          .append("circle")
            .attr("class", "actin")
            .attr('r', 5.0*self.scale)
          .merge(this.svg.selectAll(".actin"))
            //.attr('cx', function(d) {return self.x(data.data.x) + self.calcArmLength(d.x, d.y, data.data.x, data.data.y)*d.pos;})
            .attr('cx', function(d) {
              return self.calcAflowAnimation(d);
            })
            .attr('cy', function(d) {return self.y(data.data.y); })
            .attr('fill', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('stroke', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('transform', function(d) { 
              var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
              var opposite = d.y-data.data.y;
              var adjacent = d.x-data.data.x;
              var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
              if (adjacent < 0) {
                angle += 180;
              }
              return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-5/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
            });
            
        var clutches = [];
        if (showClutches) {
          for (var i = 0; i < data.data.m.length; i++) {
            if (data.data.m[i].length > 0.001) {
              var percent = data.data.m[i].length / totalModuleLength;
              //data.data.m[i].length = Math.sqrt(Math.pow(data.data.m[i].x-data.data.x,2) + Math.pow(data.data.m[i].y-data.data.y,2));
              if (percent > 0.001) {
                //data.data.m[i].c_on_rate = 1.0;
                data.data.m[i].c_on_rate = data.data.m[i].en/(percent*data.params.cpool);
                if (data.data.m[i].c_on_rate > 1.0) {
                  data.data.m[i].c_on_rate = 1.0;
                }
              }
              else {
                data.data.m[i].c_on_rate = 0.0;
              }
              var numClutchVis = percent*Math.log(Math.floor(data.params.cpool))*20;
              for (var j = 0; j < numClutchVis; j++) {
                var clutch = {};
                clutch = Object.assign(clutch, data.data.m[i]);
                clutch.pos = 1.0*(j)/(numClutchVis);
                clutch.on = data.data.m[i].c_on_rate > Math.random();
                clutches.push(clutch);
              }
            }
          }
        }


        this.svg.selectAll(".clutch")
          .data(clutches)
          .exit()
          .remove()
        this.svg.selectAll(".clutch")
          .data(clutches)
          .enter()
          .append("line")
            .attr("class", "clutch")
            .attr('stroke-width', 1)
          .merge(this.svg.selectAll(".clutch"))
            .attr('x1', function(d) {
              return self.calcAflowAnimation(d, 2, 0.5);
            })
            .attr('x2', function(d) {
              return self.calcAflowAnimation(d, 2, 0.5);
            })
            //.attr('x1', function(d) {return self.x(data.data.x) + (0.5 + d.pos*0.3)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('y1', function(d) {return self.y(data.data.y); })
            //.attr('x2', function(d) {return self.x(data.data.x) + (0.5 + d.pos*0.3)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('y2', function(d) {return self.y(data.data.y)+self.thickness/4.0; })
            .attr('fill', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('stroke', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('transform', function(d) { 
              var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
              var opposite = d.y-data.data.y;//-1;
              var adjacent = d.x-data.data.x;//1;
              var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
              if (adjacent < 0) {
                angle += 180;
              }
              return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-5/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
            })

      var substrateData = showSubstrate ? data.data.m : [];

      this.svg.selectAll(".substrate")
        .data(substrateData)
        .exit()
        .remove()
      this.svg.selectAll(".substrate")
        .data(substrateData)
        .enter()
        .append("line")
          .attr("class", "substrate")
          .attr('stroke-width', 2)
        .merge(this.svg.selectAll(".substrate"))
          .attr('x1', function(d) {return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5)-25*self.scale;})
          .attr('x2', function(d) {return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5)+25*self.scale;})
          //.attr('x1', function(d) {return self.x(data.data.x) + (0.5)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
          .attr('y1', function(d) {return self.y(data.data.y)+self.thickness/4.0; })
          //.attr('x2', function(d) {return self.x(data.data.x) + (0.5 + 0.3)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
          .attr('y2', function(d) {return self.y(data.data.y)+self.thickness/4.0; })
          .attr('fill', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
          .attr('stroke', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
          .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
          .attr('transform', function(d) { 
            var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
            var opposite = d.y-data.data.y;//-1;
            var adjacent = d.x-data.data.x;//1;
            var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
            if (adjacent < 0) {
              angle += 180;
            }
            return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-5/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
          })

        this.svg.selectAll(".substrate-spring")
          .data(substrateData)
          .exit()
          .remove()
        this.svg.selectAll(".substrate-spring")
          .data(substrateData)
          .enter()
          .append("line")
            .attr("class", "substrate-spring")
            .attr('stroke-width', 4*self.scale)
          .merge(this.svg.selectAll(".substrate-spring"))
            .attr('x1', function(d) {return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5},2,0.5)*self.scale;})
            .attr('x2', function(d) { return self.x(data.data.x) + self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('y1', function(d) {return self.y(data.data.y)+self.thickness*0.375; })
            .attr('y2', function(d) {return self.y(data.data.y)+self.thickness*0.375; })
            .attr('fill', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('stroke', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('transform', function(d) { 
              var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
              var opposite = d.y-data.data.y;//-1;
              var adjacent = d.x-data.data.x;//1;
              var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
              if (adjacent < 0) {
                angle += 180;
              }
              return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-5/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
            })

        this.svg.selectAll(".substrate-spring-connect")
          .data(substrateData)
          .exit()
          .remove()
        this.svg.selectAll(".substrate-spring-connect")
          .data(substrateData)
          .enter()
          .append("line")
            .attr("class", "substrate-spring-connect")
            .attr('stroke-width', 2)
          .merge(this.svg.selectAll(".substrate-spring-connect"))
            .attr('x1', function(d) {
              return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5}, 2, 0.5);
            })
            .attr('x2', function(d) {
              return self.calcAflowAnimation({x: d.x, y: d.y, pos: 0.5}, 2, 0.5);
            })
            //.attr('x1', function(d) {return self.x(data.data.x) + (0.5 + d.pos*0.3)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('y1', function(d) {return self.y(data.data.y)+self.thickness*0.25; })
            //.attr('x2', function(d) {return self.x(data.data.x) + (0.5 + d.pos*0.3)*self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('y2', function(d) {return self.y(data.data.y)+self.thickness*0.375; })
            .attr('fill', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('stroke', function(d) { return color(Math.sqrt(d.fx*d.fx + d.fy*d.fy)); })
            .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
            .attr('transform', function(d) { 
              var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
              var opposite = d.y-data.data.y;//-1;
              var adjacent = d.x-data.data.x;//1;
              var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
              if (adjacent < 0) {
                angle += 180;
              }
              return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-5/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
            })

      this.armgroup.selectAll("circle")
      .data([data])
      .enter()
        .append("circle")
        .attr("r", self.thickness*0.7)
        .style("stroke", "lightgrey" )
        .style("fill", "lightgrey" )
      .merge(this.armgroup.selectAll("circle"))
        //.style("stroke", function(d) { return color(data.data.fmag);} )
        //.style("fill", color(data.data.fmag))
        .attr("cx", function(d, i){return self.x(+d.data.x);})
        .attr("cy", function(d, i){return self.y(+d.data.y);});


      d3.selection.prototype.moveToFront = function() {  
          return this.each(function(){
            this.parentNode.appendChild(this);
          });
      };  

        /*this.svg.selectAll(".rect2")
        .data(data.data.m)
        .exit()
        .remove()
        this.svg.selectAll(".rect2")
        .data(data.data.m)
        .enter()
        .append("rect")
          .attr("class", "rect2")
          .attr("fill", "lightgrey")
          .attr("stroke", "lightgrey")
          //.attr("stroke-width", "100")
          .attr("rx", "5")
          .attr('height', self.thickness)
          //.attr("clip-path", "url(#round-corner)")
        .merge(this.svg.selectAll(".rect2"))
          .attr('x', function(d) {return self.x(data.data.x); })
          .attr('y', function(d) {return self.y(data.data.y); })
          .attr('fill', function(d) { return color(d.fx); })
          .attr("stroke", function(d) { return color(d.fx); })
          .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y)/5.0;})
          .attr('transform', function(d) { 
            var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y)/5.0;
            var opposite = d.y-data.data.y;//-1;
            var adjacent = d.x-data.data.x;//1;
            var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
            if (adjacent < 0) {
              angle += 180;
            }
            return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-self.thickness/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
          })*/

        var h = [];
        if (data.data.h) {
          h = data.data.h;
          /*for (var i = 0; i < data.data.h.length; i++) {
            var sumX = 0.0;
            var sumY = 0.0;
            var num = 0;
            for (var j = i-5; j < i+5; j++) {
              if (j >= 0 && j <= data.data.h.length -1) {
                sumX += data.data.h[j].x;
                sumY += data.data.h[j].y;
                num++;
              }
            }
            h.push({x: sumX/num, y: sumY/num});
          }*/
          for (var i = 0; i < h.length; i++) {
            if (i == 0) {
              h[i].v = 0.0;
            }
            else {
              var dt = h[i].time - h[i-1].time;
              h[i].v = Math.sqrt(Math.pow(h[i].x-h[i-1].x,2) + Math.pow(h[i].y-h[i-1].y,2))/dt;
            }
          }
        }

        //this.svg.selectAll(".rect1").moveToFront();

        if (config != 0) {
          this.updatePath("vl-path-area", [h]);
          this.updatePath("vl-path", [h]);
          this.updatePath("vl-path-high-velocity", [h], function(d) {return d.v > 15;});
        }

        //this.svg.selectAll("circle").moveToFront();
        this.armgroup.moveToFront();
        this.svg.selectAll(".clutch").moveToFront();
        this.svg.selectAll(".substrate").moveToFront();
        this.svg.selectAll(".substrate-spring").moveToFront();
        this.svg.selectAll(".substrate-spring-connect").moveToFront();
        this.svg.selectAll(".rect2").moveToFront();
        this.svg.selectAll(".actin").moveToFront();

        if (config == 0) {
          this.updatePath("vl-path-area", [h]);
          this.updatePath("vl-path", [h]);
          this.updatePath("vl-path-high-velocity", [h], function(d) {return d.v > 15;});
        }

    //this.svg.selectAll(".line").moveToFront();


  //console.log(data);
}


DetailedCell.prototype.updatePath = function(pathName, data, defined) {
  var self = this;
  
  this.svg.selectAll("."+pathName)
    .data(data)
    .exit()
    .remove();
  this.svg.selectAll("."+pathName)
    .data(data)
    .enter()
    .append("path")
      .attr("class","line")
      .attr("class",pathName)
      .attr("fill", "none")
      //.attr("stroke", "blue")
      //.style("opacity", 0.5)
      //.attr("stroke-width", function(d) { return 10; })
      .attr("d", function(d){
        return d3.line()
          .curve(d3.curveCardinal)
          //.curve(d3.curveBasis)
          .x(function(d) { return self.x(d.x*1000); })
          .y(function(d) { return self.y(d.y*1000); })
          (d)
        })
    .merge(this.svg.selectAll("."+pathName))
      .attr("d", function(d){
        return d3.line()
          .curve(d3.curveCardinal)
          //.curve(d3.curveBasis)
          .defined(function(d) { return defined ? defined(d): true;})
          .x(function(d) { return self.x(d.x*1000); })
          .y(function(d) { return self.y(d.y*1000); })
          (d)
        })

    d3.selection.prototype.moveToFront = function() {  
      return this.each(function(){
        this.parentNode.appendChild(this);
      });
    };  


    this.svg.selectAll("."+pathName).moveToFront();
}




