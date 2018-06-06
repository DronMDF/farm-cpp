require 'digest'
require 'minitest/autorun'
require 'score_index/score_index'

class TestScore < Minitest::Test
  def test_score_index_trivial
    index = ScoreIndex.new(
      '2017-07-19T21:24:51Z localhost 443 NOPREFIX@ffffffffffffffff',
      6
    )
    assert_equal(index.value, '2dd974')
  end
end

class TestScore < Minitest::Test
  def test_score_index_chain
    pfx = '2017-07-19T21:24:51Z localhost 443 NOPREFIX@ffffffffffffffff'
    index = ScoreIndex.new(pfx, 6)
    hash1 = Digest::SHA256.hexdigest(pfx + ' ' + index.value)
    puts hash1

    index2 = ScoreIndex.new(hash1, 6)
    hash2 = Digest::SHA256.hexdigest(hash1 + ' ' + index2.value)
    puts hash2

    index3 = ScoreIndex.new(hash2, 6)
    assert_equal(index3.value, '4288eb')
  end
end
