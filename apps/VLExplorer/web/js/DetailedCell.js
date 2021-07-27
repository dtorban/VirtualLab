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
}

DetailedCell.prototype.calcArmLength = function(x1, y1, x2, y2) {
  return Math.sqrt(Math.pow(this.x(x1)-this.x(x2),2) + Math.pow(this.y(y1)-this.y(y2),2));
}

DetailedCell.prototype.animate = function(dt) {
    var self = this;

    this.time += dt;
    
    if (self.data) {
      this.svg.selectAll(".actin")
        .attr('cx', function(d) {
          var armLength = self.calcArmLength(d.x, d.y, self.data.data.x, self.data.data.y);
          var dx = 10*self.time*self.data.data.aflow/100.0;
          dx = dx - armLength*(Math.floor(dx/armLength));
          var animationPos = armLength*d.pos - dx;
          if (animationPos < 0) {
            animationPos = animationPos + armLength;
          }
          var xVal = self.x(self.data.data.x) + animationPos;
          return Number.isNaN(xVal) ? self.x(self.data.data.x): xVal;
        });
    }

}

DetailedCell.prototype.updateData = function(data, bounds) {
  var self = this;

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

    

    this.svg.selectAll(".rect1")
      .data(data.data.m)
      .exit()
      .remove()
    this.svg.selectAll(".rect1")
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
        for (var i = 0; i < data.data.m.length; i++) {
          if (data.data.m[i].length > 0.001) {
            var percent = data.data.m[i].length / totalModuleLength;
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

        console.log(actin);

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
              var armLength = self.calcArmLength(d.x, d.y, self.data.data.x, self.data.data.y);
              var dx = 10*self.time*self.data.data.aflow/100.0;
              dx = dx - armLength*(Math.floor(dx/armLength));
              var animationPos = armLength*d.pos - dx;
              if (animationPos < 0) {
                animationPos = animationPos + armLength;
              }
              var xVal = self.x(self.data.data.x) + animationPos;
              return Number.isNaN(xVal) ? self.x(self.data.data.x): xVal;
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
            })
            

        this.svg.selectAll(".clutch")
          .data(data.data.m)
          .exit()
          .remove()
        this.svg.selectAll(".clutch")
          .data(data.data.m)
          .enter()
          .append("line")
            .attr("class", "clutch")
            .attr('height', 2)
          .merge(this.svg.selectAll(".clutch"))
            .attr('x1', function(d) {return self.x(data.data.x) + self.calcArmLength(d.x, d.y, data.data.x, data.data.y)/2.0;})
            .attr('y1', function(d) {return self.y(data.data.y); })
            .attr('x2', function(d) {return self.x(data.data.x) + self.calcArmLength(d.x, d.y, data.data.x, data.data.y)/2.0;})
            .attr('y2', function(d) {return self.y(data.data.y)+self.thickness/3.0; })
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


      this.svg.selectAll("circle")
      .data([data])
      .enter()
        .append("circle")
        .attr("r", self.thickness*0.7)
      .merge(this.svg.selectAll("circle"))
        .style("stroke", function(d) { return color(data.data.fmag);} )
        .style("fill", color(data.data.fmag))
        .attr("cx", function(d, i){return self.x(+d.data.x);})
        .attr("cy", function(d, i){return self.y(+d.data.y);});


      d3.selection.prototype.moveToFront = function() {  
          return this.each(function(){
            this.parentNode.appendChild(this);
          });
      };  

      this.svg.selectAll(".rect2").moveToFront();
      this.svg.selectAll(".clutch").moveToFront();
      this.svg.selectAll(".actin").moveToFront();
      this.svg.selectAll("circle").moveToFront();

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

        this.updatePath("vl-path-area", [h]);
        this.updatePath("vl-path", [h]);
        this.updatePath("vl-path-high-velocity", [h], function(d) {return d.v > 15;});


    //this.svg.selectAll(".line").moveToFront();


  console.log(data);
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
          .curve(d3.curveBasis)
          .x(function(d) { return self.x(d.x*1000); })
          .y(function(d) { return self.y(d.y*1000); })
          (d)
        })
    .merge(this.svg.selectAll("."+pathName))
      .attr("d", function(d){
        return d3.line()
          .curve(d3.curveBasis)
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




