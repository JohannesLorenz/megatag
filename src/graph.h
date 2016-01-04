/*************************************************************************/
/* megatag - A simple library to tag files graphically                   */
/* Copyright (C) 2015-2016                                               */
/* Johannes Lorenz (jlsf2013 @ sourceforge)                              */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 3 of the License, or (at */
/* your option) any later version.                                       */
/* This program is distributed in the hope that it will be useful, but   */
/* WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      */
/* General Public License for more details.                              */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program; if not, write to the Free Software           */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA  */
/*************************************************************************/

#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#ifndef GRAPH_H
#define GRAPH_H

template<class Vertex, class Edge, class OutEdgeList = boost::vecS, class VertexList = boost::vecS>
class graph_base_t
{
public:
	using self = graph_base_t<Vertex, Edge, OutEdgeList, VertexList>;
	using vertex = Vertex;
	using edge = Edge;
	typedef boost::adjacency_list<OutEdgeList, VertexList, boost::directedS, vertex, edge> graph_t;
protected:
	graph_t graph;
	std::string name;
	bool dump_on_exit;
public:
	typedef typename boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
	typedef typename boost::graph_traits<graph_t>::edge_descriptor edge_t;

	template<class ...Args>
	vertex_t add_vertex(Args... args)
	{
		vertex_t v_n = boost::add_vertex(graph);
		graph[v_n] = vertex(args...);
		return v_n;
	}

	template<class ...Args>
	edge_t add_edge(vertex_t v_1, vertex_t v_2, Args... args)
	{
		auto res = boost::add_edge(v_1, v_2, graph);
		if(!res.second)
		 throw "Could not add edge.";
		else
		{
			graph[res.first] = edge(args...);
			return res.first;
		}
	}


	static auto null_vertex()
		-> decltype(boost::graph_traits<graph_t>::null_vertex()) {
		return boost::graph_traits<graph_t>::null_vertex();
	}

private:
	// TODO: from stackoverflow
	static void my_replace(std::string& str, const std::string& oldStr, const std::string& newStr)
	{
		size_t pos = 0;
		while((pos = str.find(oldStr, pos)) != std::string::npos)
		{
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
	}

	template<class VertexOrEdge>
	class newline_writer
	{
		const graph_t& g;
	public:
		newline_writer(const graph_t& g) : g(g) {}
		void operator()(std::ostream &out, const VertexOrEdge& ve) const
		{
			std::string res = g[ve].label();
			my_replace(res, "\n", "\\n");
			out << "[label=\"" << res << "\"]";
		}
	};
public:
	static std::ostream& _dump_as_dot(std::ostream& stream, graph_t& g)
	{
#define USE_LABELS
#ifdef USE_LABELS

		using index_map_t = std::map<vertex_t, size_t>;
		index_map_t index_map;
		boost::associative_property_map<index_map_t> idx(index_map);

		int i=0;
		for(const vertex_t& v : vertex_container_t(g))
		 put(idx, v, i++);

		write_graphviz(stream, g,
			newline_writer<vertex_t>(g),
			newline_writer<edge_t>(g),
			boost::default_writer(), idx
			);
#else
		(void)g;
#endif
		return stream;
	}
public:
	friend std::ostream& operator<<(std::ostream &stream, const graph_base_t<vertex, edge, OutEdgeList, VertexList> &d)
	{
		return _dump_as_dot(stream, const_cast<graph_t&>(d.graph));
	}

protected:
	struct vertex_container_t // TODO: common class for those three?
	{
		const graph_t& g;
		typedef typename boost::graph_traits<graph_t>::vertex_iterator iterator;
		using const_iterator = iterator;
		const_iterator cbegin() const { return boost::vertices(g).first; } // TODO: does cons make sense?
		const_iterator cend() const { return boost::vertices(g).second; }
		iterator begin() const { return boost::vertices(g).first; } // TODO: does cons make sense?
		iterator end() const { return boost::vertices(g).second; }
		vertex_container_t(const graph_t& g) : g(g) {}
	};

	struct edge_container_t
	{
		const graph_t& g;
		typedef typename boost::graph_traits<graph_t>::edge_iterator iterator;
		using const_iterator = iterator;
		iterator begin() const { return boost::edges(g).first; } // TODO: does cons make sense?
		iterator end() const { return boost::edges(g).second; }
		edge_container_t(const graph_t& g) : g(g) {}
	};

	struct out_edge_container_t
	{
		typedef typename boost::graph_traits<graph_t>::out_edge_iterator iterator;
		using const_iterator = iterator;
		std::pair<iterator, iterator> range;
		iterator begin() const { return range.first; } // TODO: does cons make sense?
		iterator end() const { return range.second; }
		bool empty() const { return begin() == end(); }
		out_edge_container_t(const vertex_t& v, const graph_t& g) : range(boost::out_edges(v, g)) {}
	};
public:
	class safe_out_edge_iterator_t
	{
		vertex_t src_vertex;
		const graph_t& graph;
		std::size_t max, cur_edge;
		using me = safe_out_edge_iterator_t;

		std::size_t get_max() const
		{
			return boost::out_degree(src_vertex, graph); // TODO?
		}
	public:
		const me& operator++()
		{
			cur_edge = std::min(cur_edge+1, max);
			return *this;
		}
		edge_t operator*() const
		{
			out_edge_container_t oec(src_vertex, graph);
			auto itr = oec.begin();
			for(std::size_t i = 0; i < cur_edge; ++i, ++itr) ;
			return *itr;
		}
		safe_out_edge_iterator_t(const vertex_t& v, const graph_t& g, bool begin) :
			src_vertex(v),
			graph(g),
			max(get_max()),
			cur_edge(begin ? 0 : max)
		{
		}
		//! note that this will not help against edges from different graphs
		bool operator==(const me& other) const
		{
			return cur_edge == other.cur_edge;
		}
		bool operator!=(const me& other) const
		{
			return ! operator==(other);
		}
		using value_type = edge_t;
	};

	struct safe_out_edge_container_t
	{
		using me = safe_out_edge_container_t;
		using iterator = safe_out_edge_iterator_t;
		using const_iterator = iterator;

		const vertex_t& v;
		const graph_t& g;

		safe_out_edge_container_t(const vertex_t& v, const graph_t& g) :
			v(v), g(g)
		{
		}

		iterator begin() const { return safe_out_edge_iterator_t(v,g,true); } // TODO: does cons make sense?
		iterator end() const { return safe_out_edge_iterator_t(v,g,false); }
		bool empty() { auto oe = boost::out_edges(v, g); return oe.first == oe.second; }
		std::size_t size() const { return boost::out_degree(v, g); }
	};

	//! returns second == true iff the edge could be added (i.e. was not existing)
	//! useful to avoid double edges
	std::pair<edge_t, bool> try_add_edge(const vertex_t& v1, const vertex_t& v2)
	{
#ifdef USE_VEC
		if(boost::num_vertices(g) <= std::max(v1, v2))
		{
			std::cout << boost::num_vertices(g) << ", " << v1 << ", " << v2 << std::endl;
			throw "add_edge: vertex does not exist.";
		}
#endif

		auto range = boost::out_edges(v1, graph);
		auto itr = range.first;
		for( ; itr != range.second; ++itr)
		if( boost::target(*itr, graph) == v2 )
		{
			break;
		}

		std::pair<edge_t, bool> ret_val;
		ret_val.second = (itr == range.second);
		ret_val.first = (ret_val.second)
			? boost::add_edge(v1, v2, graph).first
			: *itr;
		return ret_val;
	}

	std::size_t out_degree(const vertex_t& v) { return boost::out_degree(v, graph); }

	vertex_container_t vertices() const { return vertex_container_t(graph); }
	edge_container_t edges() const { return edge_container_t(graph); }
	safe_out_edge_container_t out_edges(const vertex_t& v) const { return safe_out_edge_container_t(v, graph); }

	std::size_t num_vertices() const { return boost::num_vertices(graph); }
	std::size_t num_edges() const { return boost::num_edges(graph); }
	vertex_t add_vertex() { return boost::add_vertex(graph); }
	std::pair<edge_t, bool> add_edge(const vertex_t& v1, const vertex_t& v2) { return boost::add_edge(v1, v2, graph); }
	vertex_t target(const edge_t& e) const { return boost::target(e, graph); }
	vertex_t source(const edge_t& e) const { return boost::source(e, graph); }

	void label_edge(const edge_t& e_n) {
		std::ostringstream ss;
		ss << graph[e_n];
		graph[e_n].label = ss.str();
	}
	void label_vertex(const vertex_t& v_n) {
		std::ostringstream ss;
		ss << graph[v_n];
		graph[v_n].label = ss.str();
	}

	vertex& get(const vertex_t& v) { return graph[v]; }
	edge& get(const edge_t& e) { return graph[e]; }
	const vertex& get(const vertex_t& v) const { return graph[v]; }
	const edge& get(const edge_t& e) const { return graph[e]; }

	void transitive_closure(graph_base_t& res)
	{
		//std::map<vertex_t, vertex_t> v_map;

		typedef typename boost::property_map<graph_t, boost::vertex_index_t>::const_type
			VertexIndexMap;
		VertexIndexMap index_map = boost::get(boost::vertex_index, graph);
		typedef std::map < vertex_t, vertex_t > io_map_t;
		io_map_t vmap;
		boost::associative_property_map< io_map_t > io_map(vmap);
		//boost::transitive_closure(graph, res.graph, &io_map[0], boost::get(boost::vertex_index, graph));

		std::vector<vertex_t> to_tc_vec(num_vertices());
		boost::iterator_property_map < vertex_t *, VertexIndexMap, vertex_t, vertex_t&>
		g_to_tc_map(&to_tc_vec[0], index_map);
		boost::transitive_closure(graph, res.graph, g_to_tc_map, index_map);


		//std::cout << "map size: " << v_map.size() << std::endl;

		int i = 0;
		for(const auto& pr : to_tc_vec)
		{
//			std::cout << "mapping " << i << " to " << pr << std::endl;
			res.get(pr) = get(i);
			++i;
		}
	}

	bool has_edge(vertex_t src, vertex_t dest) { return boost::edge(src,dest,graph).second; }

	void clear() { graph.clear(); }

	graph_base_t() = default;
	graph_base_t(const graph_t& g) : graph(g) {}
};

#endif // GRAPH_H

