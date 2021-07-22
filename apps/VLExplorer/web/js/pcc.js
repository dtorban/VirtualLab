function PCChart(container, lineHover, lineClick) {
    var self = this;

	this.lineHover = lineHover;
	this.lineClick = lineClick;

	// set the dimensions and margins of the graph
	this.margin = {top: 30, right: 10, bottom: 30, left: 0};
	this.width = $( "#" + container ).width() - this.margin.left - this.margin.right;
	this.height = $( "#" + container ).height() - this.margin.top - this.margin.bottom;
	
	// append the svg object to the body of the page
	this.svg = d3.select( "#" + container)
	.append("svg")
	  .attr("width", this.width + this.margin.left + this.margin.right)
	  .attr("height", this.height + this.margin.top + this.margin.bottom)
	.append("g")
	  .attr("transform",
			"translate(" + this.margin.left + "," + this.margin.top + ")");
	
    this.y = {};

    this.x = d3.scalePoint()
        .range([0, this.width])
        .padding(1)
        .domain([]);
	// Parse the Data
	/*d3.csv("https://raw.githubusercontent.com/holtzy/D3-graph-gallery/master/DATA/iris.csv", function(data) {
        self.updateData(data);
	});*/
}

function isNumber(n) { return !isNaN(parseFloat(n)) && !isNaN(n - 0) }

PCChart.prototype.updateData = function(data) {
    var self = this;

    // Extract the list of dimensions we want to keep in the plot. Here I keep all except the column called Species
	var dimensions;
	if (data.length <= 0) {
		dimensions = [];
	}
	else {
		dimensions = d3.keys(data[0].data).filter(function(d) { return isNumber(data[0].data[d]) && d != "fx" && d != "fy" && d != "x" && d != "y"; })
	}

	
	  // For each dimension, I build a linear scale. I store all in a y object
	  for (i in dimensions) {
		var name = dimensions[i];
        //if (!(name in this.y)) {
          this.y[name] = d3.scaleLinear()
            .domain( d3.extent(data, function(d) { return +d.data[name]; }) )
            .range([this.height, 0])
        //}
        /*else {
            var ext = d3.extent(data, function(d) { return +d.data[name]; });
            ext = ext.concat(this.y[name].domain());
            this.y[name].domain( d3.extent(ext) );
        }*/
	  }
	
	  // Build the X scale -> it find the best position for each Y axis
      this.x.domain(dimensions);
	
	  // The path function take a row of the csv as input, and return x and y coordinates of the line to draw for this raw.
	  function path(d) {
		  return d3.line()(dimensions.map(function(p) { return [self.x(p), self.y[p](d.data[p])]; }));
	  }
	
      this.svg
        .selectAll(".myPath")
        .data(data)
        .exit()
        .remove();

	  // Draw the lines
	  this.svg
		.selectAll(".myPath")
		.data(data)
		.enter().append("path")
        .attr("class","myPath")
		.attr("d",  path)
		.style("fill", "none")
		.style("stroke", "#69b3a2")
		.attr("stroke-width", function(d) { return +d.chosen*2.0 + 1.5; })
        .style("opacity", function(d) { return +d.chosen > 0 ? 1.0 : 0.5; })
        .attr("stroke-width", function(d) { return +d.chosen*2.0*0.0 + 1.5; })

    this.svg
		.selectAll(".myPath")
		.attr("stroke-width", function(d) { return +d.chosen*2.0*0.0 + 1.5; })
        .style("opacity", function(d) { return +d.chosen > 0 ? 1.0 : 0.5; })
		.style("stroke", function(d) {return d.color;})
		.attr("d",  path)

	this.svg
        .selectAll(".myHoverPath")
        .data(data)
        .exit()
        .remove();

	  // Draw the lines
	  this.svg
		.selectAll(".myHoverPath")
		.data(data)
		.enter().append("path")
        .attr("class","myHoverPath")
		.attr("d",  path)
		.style("fill", "none")
		.style("stroke", "#69b3a2")
		.style("opacity", 0.0)
		.style("stroke-width","15px")
		.on("mouseenter", function(d) {
            d3.select(d3.event.target)
            if (self.lineHover) {
              self.lineHover(d.id);
            }
          })
          .on("mouseleave", function(d) {
            //console.log(+d.key,);
            d3.select(d3.event.target)
            if (self.lineHover) {
              self.lineHover(null);
            }
          })
          .on("mousedown", function(d) {
            if (self.lineClick) {
              self.lineClick(d.id);
            }
          });;

	this.svg
		.selectAll(".myHoverPath")
		.style("opacity", function(d) {return d.hover ? 0.5 : 0.0; })
		.style("stroke", function(d) {return d.color;})
		.attr("d",  path)

      this.svg.selectAll(".myAxis")
		// For each dimension of the dataset I add a 'g' element:
		.data(dimensions).exit()
        .remove();
	
	  // Draw the axis:
	  this.svg.selectAll(".myAxis")
		// For each dimension of the dataset I add a 'g' element:
		.data(dimensions).enter()
		.append("g")
        .attr("class","myAxis")
		// I translate this element to its right position on the x axis
		.attr("transform", function(d) { return "translate(" + self.x(d) + ")"; })
		// And I build the axis with the call function
		.each(function(d) { d3.select(this).call(d3.axisLeft().scale(self.y[d])); })
		// Add axis title
		.append("text")
		  .attr("class","axis-label")
		  .style("text-anchor", "middle")
		  .attr("y", -9)
		  .text(function(d) { return d; })
		  .style("fill", "black")

      this.svg.selectAll(".myAxis")
          // I translate this element to its right position on the x axis
          .attr("transform", function(d) { return "translate(" + self.x(d) + ")"; })
          // And I build the axis with the call function
          .each(function(d) { d3.select(this).call(d3.axisLeft().scale(self.y[d]).tickFormat(d3.format(",.2f"))); })

	
	  this.svg.selectAll(".myAxis")
	  	.each(function(d) { d3.select(this).select(".axis-label").text(function(d) { return d.replace("_mean",""); }); })


}
 
//Read the data
/*d3.csv("https://raw.githubusercontent.com/holtzy/D3-graph-gallery/master/DATA/iris.csv", function(data) {
    //updateData(data, "Sepal_Length", "Petal_Length");
    updateData(this, data, "Sepal_Length", "Petal_Length");

});*/