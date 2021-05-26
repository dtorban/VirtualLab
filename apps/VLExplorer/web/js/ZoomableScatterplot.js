function ZoomableScatterplot(container, margin = {top: 10, right: 30, bottom: 30, left: 60}) {
    this.colors = ['red','blue','green','yellow','orange','purple','black'];
    this.containerId = "#" + container;
    this.container = container;

    // set the dimensions and margins of the graph
    this.margin = margin;
    this.width = $( this.containerId ).width() - this.margin.left - this.margin.right,
    this.height = $( this.containerId ).height() - this.margin.top - this.margin.bottom;
    console.log(this.containerId, this.width, this.height);
    
    // append the SVG object to the body of the page
    this.SVG = d3.select(this.containerId)
        .append("svg")
        .attr("width", this.width + this.margin.left + this.margin.right)
        .attr("height", this.height + this.margin.top + this.margin.bottom)
        .append("g")
        .attr("transform",
                "translate(" + this.margin.left + "," + this.margin.top + ")");

    this.x = d3.scaleLinear()
        .domain([0,1])
        .range([ 0, this.width ]);
    this.xAxis = this.SVG.append("g")
        .attr("transform", "translate(0," + this.height + ")")
        .call(d3.axisBottom(this.x));

    this.y = d3.scaleLinear()
        .domain([0,1])
        .range([ this.height, 0]);
    this.yAxis = this.SVG.append("g")
        .call(d3.axisLeft(this.y));

    // Add a clipPath: everything out of this area won't be drawn.
    this.clip = this.SVG.append("defs").append("SVG:clipPath")
        .attr("id", this.container+"-clip")
        .append("SVG:rect")
        .attr("width", this.width )
        .attr("height", this.height )
        .attr("x", 0)
        .attr("y", 0);

    // Create the scatter variable: where both the circles and the brush take place
    this.scatter = this.SVG.append('g')
        .attr("clip-path", "url(#"+this.container+"-clip)")

    // Set the zoom and Pan features: how much you can zoom, on which part, and what to do when there is a zoom
    this.zoom = d3.zoom()
        .scaleExtent([.5, 20])  // This control how much you can unzoom (x0.5) and zoom (x20)
        .extent([[0, 0], [this.width, this.height]])

    // This add an invisible rect on top of the chart area. This rect can recover pointer events: necessary to understand when the user zoom
    var zoomElem = this.SVG.append("rect")
        .attr("width", this.width)
        .attr("height", this.height)
        .style("fill", "none")
        .style("pointer-events", "all")
        //.attr('transform', 'translate(' + this.margin.left + ',' + this.margin.top + ')')
        .call(this.zoom);

    this.zoomX = this.x;
    this.zoomY = this.y;

    //return {scatter: scatter, x:x, y:y, zoom:zoom, xAxis:xAxis, yAxis:yAxis, zoomElem:zoomElem, zoomX:null, zoomY:null, zooming:false};
}

ZoomableScatterplot.prototype.updateData = function(data,a,b,color) {
    var self = this;
    // Add Y axis
    this.x.domain(d3.extent(data, function(d) { return a(d); }));
    this.y.domain(d3.extent(data, function(d) { return b(d); }));

    if (!(this.zoomX && this.zoomY)) {
    this.zoomX = this.x;
    this.zoomY = this.y;
    }

    this.xAxis.call(d3.axisBottom(this.zoomX))
    this.yAxis.call(d3.axisLeft(this.zoomY))

    // Add circles
    this.scatter
        .selectAll("circle")
        .data(data)
        .enter()
        .append("circle")
        .attr("cx", function (d) { return self.zoomX(a(d)); } )
        .attr("cy", function (d) { return self.zoomY(b(d)); } )
        //.attr("cy", function (d) { return y(d["Petal_Length"]); } )
        .attr("r", 3)
        //.style("fill", "#61a3a9")
        .style("fill", function (d) { return self.colors[color(d)%self.colors.length];})
        .style("opacity", 1.0)

    this.scatter
        .selectAll("circle")
        .attr('cx', function(d) {return self.zoomX(a(d))})
        .attr('cy', function(d) {return self.zoomY(b(d))})
        .style("fill", function (d) { return self.colors[color(d)%self.colors.length];});

    /*this.scatter
    .selectAll("circle")
    .data(data)
    .transition()
    .delay(0)
    .attr("cx", function (d) { return this.zoomX(a(d)); } )
    .attr("cy", function (d) { return this.zoomY(b(d)); } )*/

    this.zoom.on("zoom", updateChart);
    /*this.zoomElem.transition()
    .duration(750)
    .call(this.zoom.transform, d3.zoomIdentity);*/

    // now the user can zoom and it will trigger the function called updateChart

    // A function that updates the chart when the user zoom and thus new boundaries are available
    function updateChart() {
    
        // recover the new scale
        var newX = d3.event.transform.rescaleX(self.x);
        var newY = d3.event.transform.rescaleY(self.y);

        self.zoomX = newX;
        self.zoomY = newY;

        // update axes with these new boundaries
        self.xAxis.call(d3.axisBottom(newX))
        self.yAxis.call(d3.axisLeft(newY))

        // update circle position
        self.scatter
            .selectAll("circle")
            .attr('cx', function(d) {return newX(a(d))})
            .attr('cy', function(d) {return newY(b(d))})
            .style("fill", function (d) { return self.colors[color(d)%self.colors.length];});
        }
}
 
//Read the data
/*d3.csv("https://raw.githubusercontent.com/holtzy/D3-graph-gallery/master/DATA/iris.csv", function(data) {
    //updateData(data, "Sepal_Length", "Petal_Length");
    updateData(this, data, "Sepal_Length", "Petal_Length");

});*/